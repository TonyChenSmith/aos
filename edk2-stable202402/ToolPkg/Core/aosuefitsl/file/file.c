/*
 * AOS Loader文件操作。
 * 2024-10-15创建。 
 * 
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "file/aosfile.h"

/*
 * 需要在引导器环节加载的内核文件。每个索引固定了一个对应的专门模块。
 */
CHAR16* aos_boot_files[]={
	L"aos\\boot\\init.aem",
	L"aos\\boot\\memory.aem",
	L"aos\\boot\\util.aem"
};

/*根目录*/
static EFI_FILE_HANDLE esp_root=NULL;

/*
 * 获取当前启动区根目录。这个同时会打开卷，运行前应假设esp_root为空。
 * 不为空的esp_root不需要获得，定位应该是唯一的。
 */
static
EFI_STATUS
EFIAPI
aos_get_esp_root(VOID)
{
	EFI_STATUS status;
	EFI_LOADED_IMAGE_PROTOCOL* image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simple;

	if(esp_root!=NULL)
	{
		return EFI_SUCCESS;
	}

	/*获取加载程序映像协议，其应该有简单文件系统协议*/
	status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&image);
	if(EFI_ERROR(status))
	{
		return status;
	}
	EFI_DEVICE_PATH_PROTOCOL* boot;
	status=gBS->HandleProtocol(image->DeviceHandle,&gEfiDevicePathProtocolGuid,(VOID*)&boot);
	if(EFI_ERROR(status))
	{
		return status;
	}

	if(boot_params.boot_device==NULL)
	{
		UINTN size=GetDevicePathSize(boot);
		if(size==0)
		{
			/*设备分析出错*/
			return EFI_DEVICE_ERROR;
		}
		boot_params.boot_device=aos_allocate_pool(size);
		CopyMem(boot_params.boot_device,(VOID*)boot,size);
	}
	status=gBS->HandleProtocol(image->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&simple);
	if(EFI_ERROR(status))
	{
		return status;
	}
	return simple->OpenVolume(simple,&esp_root);
}

/*
 * 加载引导器模块。加载引导器的核心是分析内核文件，内核文件是使用特制（目前还没有）的ELF64文件。
 * 其使用SystemV ABI和PIE，我们在这一环节需要检查文件的完整性和合理性（待实现）。
 */
static
EFI_STATUS
EFIAPI
aos_load_bootstrap_module(
	IN UINTN index,
	IN UINT8* file,
	IN UINTN size
)
{
	/*检查文件头，应该为x86_64 SystemV ABI的小端序ELF文件*/
	if(aos_read_32(file,EI_MAG)!=ELF_MAGIC||file[EI_CLASS]!=ELF_CLASS_64||file[EI_DATA]!=ELF_DATA_LSB||file[EI_OSABI]!=ELF_SYSTEMV_ABI||aos_read_16(file,E_TYPE)!=ELF_TYPE_DYN||aos_read_16(file,E_MACHINE)!=ELF_MACHINE_AMD64||aos_read_32(file,E_VERSION)!=ELF_VERSION_CURRENT||aos_read_16(file,E_EHSIZE)!=ELF_EHSIZE_64)
	{
		/*文件头错误，反馈为加载错误*/
		return EFI_LOAD_ERROR;
	}
	/*成功判断，开始获取程序头表*/
	boot_params.modules[index].entry=aos_read_64(file,E_ENTRY);
	UINTN poffset=aos_read_64(file,E_PHOFF);
	UINT16 pcount=aos_read_16(file,E_PHNUM);
	UINTN pages=0;
	UINTN ppointer=poffset;
	/*循环搜索，搜索LOAD节，其按页对齐*/
	for(UINT16 pi=0;pi<pcount;pi++)
	{
		if(aos_read_32(file,ppointer+P_TYPE)==ELF_P_TYPE_LOAD)
		{
			UINTN ppages=aos_size_to_pages((aos_read_64(file,ppointer+P_VADDR)&0xFFF)+aos_read_64(file,ppointer+P_MEMSZ));
			pages=pages+ppages;
		}
		ppointer=ppointer+56;
	}
	/*申请页面*/
	EFI_STATUS status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,pages,&boot_params.modules[index].base);
	if(EFI_ERROR(status))
	{
		return status;
	}
	boot_params.modules[index].pages=pages;
	ZeroMem((VOID*)boot_params.modules[index].base,aos_pages_to_size(pages));
	/*写入内存*/
	ppointer=poffset;
	for(UINT16 pi=0;pi<pcount;pi++)
	{
		if(aos_read_32(file,ppointer+P_TYPE)==ELF_P_TYPE_LOAD)
		{
			UINTN phoff=aos_read_64(file,ppointer+P_OFFSET);
			UINTN phstart=aos_read_64(file,ppointer+P_VADDR);
			UINTN phsize=aos_read_64(file,ppointer+P_FILESZ);
			CopyMem((VOID*)(boot_params.modules[index].base+phstart),&file[phoff],phsize);
		}
		ppointer=ppointer+56;
	}
	return EFI_SUCCESS;
}

/*
 * 加载引导程序到内存上。
 */
EFI_STATUS
EFIAPI
aos_load_bootstrap(VOID)
{
	EFI_STATUS status;
	status=aos_get_esp_root();
	if(EFI_ERROR(status))
	{
		return status;
	}

	UINT8* buffer=NULL;
	UINTN buffer_size=0;

	/*循环载入每一个文件*/
	for(UINTN index=0;index<AOS_BOOT_MODULE_COUNT;index++)
	{
		UINTN bsize=0;
		EFI_FILE_INFO* file_info=NULL;
		EFI_FILE_HANDLE file;
		status=esp_root->Open(esp_root,&file,aos_boot_files[index],EFI_FILE_MODE_READ,0);
		if(status==EFI_NOT_FOUND)
		{
			return status;
		}
		else if(EFI_ERROR(status))
		{
			return status;
		}
		status=file->GetInfo(file,&gEfiFileInfoGuid,&bsize,file_info);
		if(status==EFI_BUFFER_TOO_SMALL)
		{
			status=gBS->AllocatePool(EfiBootServicesData,bsize,(VOID*)&file_info);
			if(EFI_ERROR(status))
			{
				return status;
			}
			status=file->GetInfo(file,&gEfiFileInfoGuid,&bsize,file_info);
			if(EFI_ERROR(status))
			{
				return status;
			}
			if(buffer_size<file_info->FileSize)
			{
				buffer=(UINT8*)ReallocatePool(buffer_size,file_info->FileSize,buffer);
				if(buffer==NULL)
				{
					return EFI_OUT_OF_RESOURCES;
				}
				buffer_size=file_info->FileSize;
			}
			status=file->Read(file,&file_info->FileSize,buffer);
			if(EFI_ERROR(status))
			{
				return status;
			}
			status=aos_load_bootstrap_module(index,buffer,file_info->FileSize);
			if(EFI_ERROR(status))
			{
				return status;
			}
			gBS->FreePool(file_info);
		}
		else if(EFI_ERROR(status))
		{
			return status;
		}
	}
	gBS->FreePool(buffer);
	esp_root->Close(esp_root);
	esp_root=NULL;
	return EFI_SUCCESS;
}
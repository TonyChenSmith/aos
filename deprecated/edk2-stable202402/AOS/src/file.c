/*
 * 文件操作相关实现。主要操作启动核模块文件。
 * @date 2025-01-25
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * 
 * SPDX-License-Identifier: MIT
 */
#include <file.h>

/*启动核模块文件列表*/
STATIC CHAR16* module_files[BOOT_MODULE_COUNT];

/*
 * 加载启动核模块。加载引导器的核心是分析内核文件，内核文件是使用特制（目前还没有）的ELF64文件。
 * 其使用SystemV ABI和PIE，我们在这一环节需要检查文件的完整性和合理性（待实现）。
 * 
 * @param params 启动核参数。
 * @param index	 模块索引。
 * @param file	 模块文件内容。
 * @param size	 模块文件长度。
 * 
 * @return 状态码，出错返回非零值。
 */
STATIC EFI_STATUS EFIAPI uefi_load_module(IN boot_params* restrict params,IN UINTN index,IN UINT8* file,IN UINTN size)
{
	/*检查文件头，应该为x86_64 SystemV ABI的小端序ELF文件*/
	if(elf_read_32(file,EI_MAG)!=ELF_MAGIC||file[EI_CLASS]!=ELF_CLASS_64||file[EI_DATA]!=ELF_DATA_LSB||file[EI_OSABI]!=ELF_SYSTEMV_ABI||elf_read_16(file,E_TYPE)!=ELF_TYPE_DYN||elf_read_16(file,E_MACHINE)!=ELF_MACHINE_AMD64||elf_read_32(file,E_VERSION)!=ELF_VERSION_CURRENT||elf_read_16(file,E_EHSIZE)!=ELF_EHSIZE_64)
	{
		/*文件头错误，反馈为加载错误*/
		return EFI_LOAD_ERROR;
	}

	params->modules[index].entry=elf_read_64(file,E_ENTRY);

	/*获取程序头表*/
	UINTN poffset=elf_read_64(file,E_PHOFF);
	UINT16 pcount=elf_read_16(file,E_PHNUM);
	UINTN pages=0;
	UINTN ppointer=poffset;

	/*搜索LOAD节，其按页对齐*/
	for(UINT16 pi=0;pi<pcount;pi++)
	{
		if(elf_read_32(file,ppointer+P_TYPE)==ELF_P_TYPE_LOAD)
		{
			UINTN ppages=EFI_SIZE_TO_PAGES((elf_read_64(file,ppointer+P_VADDR)&0xFFF)+elf_read_64(file,ppointer+P_MEMSZ));
			pages=pages+ppages;
		}
		ppointer=ppointer+56;
	}

	/*申请页面并清空*/
	EFI_STATUS status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,pages,&params->modules[index].base);
	if(EFI_ERROR(status))
	{
		return status;
	}
	params->modules[index].pages=pages;
	ZeroMem((VOID*)params->modules[index].base,EFI_PAGES_TO_SIZE(pages));

	/*写入内存*/
	ppointer=poffset;
	for(UINT16 pi=0;pi<pcount;pi++)
	{
		if(elf_read_32(file,ppointer+P_TYPE)==ELF_P_TYPE_LOAD)
		{
			UINTN phoff=elf_read_64(file,ppointer+P_OFFSET);
			UINTN phstart=elf_read_64(file,ppointer+P_VADDR);
			UINTN phsize=elf_read_64(file,ppointer+P_FILESZ);
			CopyMem((VOID*)(params->modules[index].base+phstart),&file[phoff],phsize);
		}
		ppointer=ppointer+56;
	}
	return EFI_SUCCESS;
}

/*
 * 初始化启动核，将启动核模块加载进入内存中。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_load_boot(IN boot_params* restrict params)
{
	EFI_STATUS status;

	/*初始化文件列表*/
	module_files[BOOT_INIT_MODULE]=BOOT_INIT_FILE;
	module_files[BOOT_MEMORY_MODULE]=BOOT_MEMORY_FILE;
	module_files[BOOT_UTIL_MODULE]=BOOT_UTIL_FILE;

	/*获取加载程序映像协议，其应该有简单文件系统协议*/
	EFI_FILE_HANDLE esp_root;
	EFI_LOADED_IMAGE_PROTOCOL* image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simple;
	status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&image);
	if(EFI_ERROR(status))
	{
		return status;
	}
	status=gBS->HandleProtocol(image->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&simple);
	if(EFI_ERROR(status))
	{
		return status;
	}
	status=simple->OpenVolume(simple,&esp_root);
	if(EFI_ERROR(status))
	{
		return status;
	}

	/*数据缓冲*/
	UINT8* buffer=NULL;
	UINTN buffer_size=0;

	/*循环载入每一个模块*/
	for(UINTN index=0;index<BOOT_MODULE_COUNT;index++)
	{
		UINTN bsize=0;
		EFI_FILE_INFO* file_info=NULL;
		EFI_FILE_HANDLE file;
		status=esp_root->Open(esp_root,&file,module_files[index],EFI_FILE_MODE_READ,0);
		if(EFI_ERROR(status))
		{
			return status;
		}

		/*获取文件信息*/
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
			status=uefi_load_module(params,index,buffer,file_info->FileSize);
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
	return EFI_SUCCESS;
}
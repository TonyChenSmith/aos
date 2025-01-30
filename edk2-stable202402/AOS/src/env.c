/*
 * 运行环境操作相关实现。
 * @date 2025-01-25
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * 
 * SPDX-License-Identifier: MIT
 */
#include <env.h>

/*
 * 初始化固定内存块所需信息，包括启动核参数和设备路径。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_init_fixed(boot_params** restrict params)
{
	UINTN size=sizeof(boot_params);
	EFI_STATUS status;

	/*获取设备路径长度*/
	EFI_LOADED_IMAGE_PROTOCOL* image;
	EFI_DEVICE_PATH_PROTOCOL* boot_device;
	status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&image);
	if(EFI_ERROR(status))
	{
		return status;
	}
	status=gBS->HandleProtocol(image->DeviceHandle,&gEfiDevicePathProtocolGuid,(VOID*)&boot_device);
	if(EFI_ERROR(status))
	{
		return status;
	}
	UINTN device_size=GetDevicePathSize(boot_device);
	if(device_size==0)
	{
		/*设备路径出现问题*/
		return EFI_DEVICE_ERROR;
	}
	size=size+device_size;

	/*申请固定内存块*/
	UINTN base;
	UINTN pages=EFI_SIZE_TO_PAGES(size);
	status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,pages,&base);
	if(EFI_ERROR(status))
	{
		return status;
	}
	ZeroMem((VOID*)base,pages);
	*params=(boot_params*)base;
	(*params)->boot_device=(VOID*)(base+sizeof(boot_params));
	CopyMem((*params)->boot_device,boot_device,device_size);

	/*记录内存块*/
	(*params)->block[BOOT_FIXED_BLOCK].base=base;
	(*params)->block[BOOT_FIXED_BLOCK].pages=pages;
	
	return EFI_SUCCESS;
}

/*
 * 设置系统表，包括ACPI、SMBIOS和运行时服务。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_set_table(boot_params* restrict params)
{
	EFI_STATUS status;

	/*获取ACPI 2.0表*/
	params->acpi=NULL;
	status=EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid,&params->acpi);
	if(EFI_ERROR(status))
	{
		status=EfiGetSystemConfigurationTable(&gEfiAcpi10TableGuid,&params->acpi);
		if(EFI_ERROR(status))
		{
			/*不能找不到ACPI表，说明不兼容*/
			return status;
		}
	}
	params->smbios=NULL;
	params->smbios3=NULL;
	EfiGetSystemConfigurationTable(&gEfiSmbiosTableGuid,&params->smbios);
	EfiGetSystemConfigurationTable(&gEfiSmbios3TableGuid,&params->smbios3);

	params->runtime=gRT;

	return EFI_SUCCESS;
}

/*
 * 获取系统具有核心数与可用核心数。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，总是成功。
 */
EFI_STATUS EFIAPI uefi_get_enable_cores(boot_params* restrict params)
{
	EFI_STATUS status;

	/*第一步，尝试获取多核服务*/
	EFI_MP_SERVICES_PROTOCOL* mp;
	status=gBS->LocateProtocol(&gEfiMpServiceProtocolGuid,NULL,(VOID**)&mp);
	if(!EFI_ERROR(status))
	{
		UINTN proc;
		UINTN enable;
		status=mp->GetNumberOfProcessors(mp,&proc,&enable);
		params->env.processors=(UINT32)proc;
		params->env.enable_processors=(UINT32)enable;
		return status;
	}
	
	/*第一步失败，尝试获取MADT*/
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp=params->acpi;
	EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* madt=NULL;
	if(rsdp->Revision==1)
	{
		EFI_ACPI_DESCRIPTION_HEADER* table=(EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)rsdp->RsdtAddress);
		UINT32 size=(table->Length-sizeof(EFI_ACPI_DESCRIPTION_HEADER))>>2;
		UINT32* entry=(UINT32*)((UINTN)table+sizeof(EFI_ACPI_DESCRIPTION_HEADER));
		for(UINT32 index=0;index<size;index++)
		{
			if(((EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)entry[index]))->Signature==SIGNATURE_32('A','P','I','C'))
			{
				madt=(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)((UINTN)entry[index]);
				break;
			}
		}
	}
	else
	{
		if(rsdp->XsdtAddress)
		{
			EFI_ACPI_DESCRIPTION_HEADER* table=(EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)rsdp->XsdtAddress);
			UINT32 size=(table->Length-sizeof(EFI_ACPI_DESCRIPTION_HEADER))>>3;
			UINT64* entry=(UINT64*)((UINTN)table+sizeof(EFI_ACPI_DESCRIPTION_HEADER));
			for(UINT32 index=0;index<size;index++)
			{
				if(((EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)entry[index]))->Signature==SIGNATURE_32('A','P','I','C'))
				{
					madt=(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)((UINTN)entry[index]);
					break;
				}
			}
		}
		else
		{
			EFI_ACPI_DESCRIPTION_HEADER* table=(EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)rsdp->RsdtAddress);
			UINT32 size=(table->Length-sizeof(EFI_ACPI_DESCRIPTION_HEADER))>>2;
			UINT32* entry=(UINT32*)((UINTN)table+sizeof(EFI_ACPI_DESCRIPTION_HEADER));
			for(UINT32 index=0;index<size;index++)
			{
				if(((EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)entry[index]))->Signature==SIGNATURE_32('A','P','I','C'))
				{
					madt=(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)((UINTN)entry[index]);
					break;
				}
			}
		}
	}

	if(madt==NULL)
	{
		/*找不到MADT，默认单核*/
		params->env.processors=1;
		params->env.enable_processors=1;
		return EFI_SUCCESS;
	}

	params->env.processors=0;
	params->env.enable_processors=0;
	
	/*遍历MADT中的子表，记录LAPIC，忽视出错和重复的可能*/
	UINT8* entry=(UINT8*)((UINTN)madt+sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
	UINT32 total=madt->Header.Length;
	UINT32 offset=sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);
	while(offset<total)
	{
		if(entry[0]==EFI_ACPI_4_0_PROCESSOR_LOCAL_APIC)
		{
			params->env.processors++;
			EFI_ACPI_4_0_PROCESSOR_LOCAL_APIC_STRUCTURE* lapic=(EFI_ACPI_4_0_PROCESSOR_LOCAL_APIC_STRUCTURE*)entry;
			if(lapic->Flags)
			{
				params->env.enable_processors++;
			}
		}
		else if(entry[0]==EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC)
		{
			params->env.processors++;
			EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE* lapic=(EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE*)entry;
			if(lapic->Flags)
			{
				params->env.enable_processors++;
			}
		}

		offset=offset+entry[1];
		entry=(UINT8*)((UINTN)entry+entry[1]);
	}

	if(!params->env.enable_processors)
	{
		/*表格完全出错*/
		params->env.processors=1;
		params->env.enable_processors=1;
	}

	return EFI_SUCCESS;
}

/*
 * 设置PMMS内存块和内存映射内存块。
 *
 * @param params 启动核参数。
 * @param key	 映射键。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_set_memory_map(boot_params* restrict params,OUT UINTN* key)
{
	EFI_STATUS status;
	UINTN entry_size;
	UINT32 version;
	UINTN map_key;
	EFI_MEMORY_DESCRIPTOR* memmap=NULL;
	UINTN map_size=0;

	/*
	 * 第一步，申请初始PMMS内存块，固定为1MB，对应于21760个可分配结点。
	 * 理应在进入虚拟内存空间前足够分配内存。
	 */
	params->block[BOOT_PMMS_BLOCK].pages=SIZE_1MB>>12;
	status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,params->block[BOOT_PMMS_BLOCK].pages,&params->block[BOOT_PMMS_BLOCK].base);
	if(EFI_ERROR(status))
	{
		return status;
	}

	/*第二步，获取内存映射所需大小*/
	status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&entry_size,&version);
	if(status==EFI_BUFFER_TOO_SMALL)
	{
		ASSERT(map_size>0);
		/*预留4个空位，避免越界*/
		params->block[BOOT_MEMORY_MAP_BLOCK].pages=EFI_SIZE_TO_PAGES(map_size+(sizeof(EFI_MEMORY_DESCRIPTOR)<<2));
		status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,params->block[BOOT_MEMORY_MAP_BLOCK].pages,&params->block[BOOT_MEMORY_MAP_BLOCK].base);
		if(EFI_ERROR(status))
		{
			return status;
		}

		map_size=EFI_PAGES_TO_SIZE(params->block[BOOT_MEMORY_MAP_BLOCK].pages);
		memmap=(EFI_MEMORY_DESCRIPTOR*)params->block[BOOT_MEMORY_MAP_BLOCK].base;
		status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&entry_size,&version);
		ASSERT(status==EFI_SUCCESS);
		ASSERT(version==EFI_MEMORY_DESCRIPTOR_VERSION);

		*key=map_key;
		params->env.memmap_length=map_size;
		params->env.memmap=memmap;
		params->env.entry_size=entry_size;

		/*调试时输出内存信息*/
		DEBUG((DEBUG_INFO,"=== Memory Map ===\n"));
		EFI_MEMORY_DESCRIPTOR* dsc=memmap;
		UINTN offset=entry_size;
		UINTN end=map_size+(UINTN)dsc;
		while((UINTN)dsc<end)
		{
			DEBUG((DEBUG_INFO,"0x%lX-0x%lX,page=%lu,type=%lu.\n",dsc->PhysicalStart,dsc->PhysicalStart+(dsc->NumberOfPages<<12)-1,dsc->NumberOfPages,dsc->Type));
			dsc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)dsc+offset);
		}

		return EFI_SUCCESS;
	}
	return status;
}
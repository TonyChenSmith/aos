/*
 * aos.uefi模块流程。实现了在UEFI阶段的启动流程。
 * @date 2025-01-24
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * 
 * SPDX-License-Identifier: MIT
 */
#include <module.h>

/*启动核参数*/
STATIC boot_params* params;

/*
 * 模块入口函数。
 *
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 无返回值。
 */
EFI_STATUS EFIAPI uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table)
{
	EFI_STATUS status;
	DEBUG((DEBUG_INFO,"=== Module aos.uefi ===\n"));

	/*关闭看门狗*/
	status=gBS->SetWatchdogTimer(0,0,0,NULL);
	if(EFI_ERROR(status))
	{
		/*错误：关闭看门狗失败。*/
		DEBUG((DEBUG_ERROR,"Error: Failed to shut down the watchdog.\n"));
		return status;
	}

	/*初始化固定内存块*/
	status=uefi_init_fixed(&params);
	if(EFI_ERROR(status))
	{
		/*错误：初始化启动核参数失败。*/
		DEBUG((DEBUG_ERROR,"Error: Failed to initialize boot kernel parameters.\n"));
		return status;
	}

	/*加载启动核模块*/
	status=uefi_load_boot(params);
	if(EFI_ERROR(status))
	{
		/*错误：加载启动核模块失败。*/
		DEBUG((DEBUG_ERROR,"Error: Failed to load the boot kernel module.\n"));
		return status;
	}

	/*初始化设备图像信息*/
	status=uefi_init_graphics(params);
	if(EFI_ERROR(status))
	{
		/*错误：设置图像信息失败。*/
		DEBUG((DEBUG_ERROR,"Error: Failed to set image information.\n"));
		return status;
	}

	/*设置系统表*/
	status=uefi_set_table(params);
	if(EFI_ERROR(status))
	{
		/*错误：未找到ACPI表。*/
		DEBUG((DEBUG_ERROR,"Error: No ACPI tables found.\n"));
		return status;
	}

	/*获取活动核心数目*/
	uefi_get_enable_cores(params);

	/*初始化内存映射和PMMS块*/
	UINTN key;
	status=uefi_set_memory_map(params,&key);
	if(EFI_ERROR(status))
	{
		/*错误：系统可用物理内存空间不足。*/
		DEBUG((DEBUG_ERROR,"Error: Insufficient available physical memory in the system.\n"));
		return status;
	}

	/*结束启动服务*/
	status=gBS->ExitBootServices(image_handle,key);
	ASSERT(status==EFI_SUCCESS);

	/*输出启动核参数*/
	DEBUG((DEBUG_INFO,"=== Boot Kernel Parameters ===\n"));
	DEBUG((DEBUG_INFO,"blocks[BOOT_FIXED_BLOCK]:base=0x%lX,pages=%lu\n",params->blocks[BOOT_FIXED_BLOCK].base,params->blocks[BOOT_FIXED_BLOCK].pages));
	DEBUG((DEBUG_INFO,"blocks[BOOT_MEMORY_MAP_BLOCK]:base=0x%lX,pages=%lu\n",params->blocks[BOOT_MEMORY_MAP_BLOCK].base,params->blocks[BOOT_MEMORY_MAP_BLOCK].pages));
	DEBUG((DEBUG_INFO,"blocks[BOOT_PMMS_BLOCK]:base=0x%lX,pages=%lu\n",params->blocks[BOOT_PMMS_BLOCK].base,params->blocks[BOOT_PMMS_BLOCK].pages));
	boot_graphics_info* info=&params->graphics_info;
	DEBUG((DEBUG_INFO,"graphics_info:horizontal_resolution=%u,vertical_resolution=%u,frame_buffer_base=0x%lX,frame_buffer_size=%lu,red_mask=0x%X,green_mask=0x%X,blue_mask=0x%X,reserved_mask=0x%X,scan_line_length=%u\n",info->horizontal_resolution,info->vertical_resolution,info->frame_buffer_base,info->frame_buffer_size,info->red_mask,info->green_mask,info->blue_mask,info->reserved_mask,info->scan_line_length));
	DEBUG((DEBUG_INFO,"env:processors=%lu,enable_processors=%lu,memmap=0x%lX,entry_size=%lu,memmap_length=%lu\n",params->env.processors,params->env.enable_processors,params->env.memmap,params->env.entry_size,params->env.memmap_length));
	DEBUG((DEBUG_INFO,"acpi=0x%lX\n",params->acpi));
	DEBUG((DEBUG_INFO,"smbios=0x%lX\n",params->smbios));
	DEBUG((DEBUG_INFO,"smbios3=0x%lX\n",params->smbios3));
	DEBUG((DEBUG_INFO,"runtime=0x%lX\n",params->runtime));
	DEBUG((DEBUG_INFO,"boot_device=0x%lX\n",params->boot_device));
	DEBUG((DEBUG_INFO,"module[BOOT_INIT_MODULE]:base=0x%lX,entry=0x%lX,pages=%lu\n",params->modules[BOOT_INIT_MODULE].base,params->modules[BOOT_INIT_MODULE].entry,params->modules[BOOT_INIT_MODULE].pages));
	DEBUG((DEBUG_INFO,"module[BOOT_MEMORY_MODULE]:base=0x%lX,entry=0x%lX,pages=%lu\n",params->modules[BOOT_MEMORY_MODULE].base,params->modules[BOOT_MEMORY_MODULE].entry,params->modules[BOOT_MEMORY_MODULE].pages));
	DEBUG((DEBUG_INFO,"module[BOOT_UTIL_MODULE]:base=0x%lX,entry=0x%lX,pages=%lu\n",params->modules[BOOT_UTIL_MODULE].base,params->modules[BOOT_UTIL_MODULE].entry,params->modules[BOOT_UTIL_MODULE].pages));

	/*进入启动核初始化模块入口函数*/
	aos_boot_trampoline trampoline=(aos_boot_trampoline)(params->modules[BOOT_INIT_MODULE].entry+params->modules[BOOT_INIT_MODULE].base);
	trampoline(params);

	return EFI_SUCCESS;
}
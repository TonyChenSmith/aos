/*
 * 使用类似于linux内核的命名方式。
 * AOS UEFI Bootloader主入口程序。
 * 2024-05-28创建。
 */
#include "aosloader.h"

/*启动参数*/
AOS_BOOT_PARAMS boot_params;

/*
 * 入口函数。理论上不会返回。
 * @param image_handle 程序句柄。
 * @param system_table 系统表。
 */
EFI_STATUS
EFIAPI
aos_uefi_entry(
	IN EFI_HANDLE        image_handle,
	IN EFI_SYSTEM_TABLE* system_table
)
{
	EFI_STATUS status; /*状态*/

	DEBUG((DEBUG_INFO,"===AOS UEFI TSL Module===\n"));

	/*关闭一下看门狗*/
	status=gBS->SetWatchdogTimer(0,0,0,NULL);
	if(status==EFI_DEVICE_ERROR)
	{
		/*出现看门狗硬件错误*/
		DEBUG((DEBUG_ERROR,"Failed to close the watchdog.\nError code:%u.\n",status));
		return status;
	}

	/*初始化启动参数*/
	status=aos_init_cpu_info();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"CPU not supported.\nError code:%u.\n",status));
		return status;
	}

	/*预分配内存池，假设足够使用*/
	status=aos_init_memory();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Failed to preallocate memory.\nError code:%u.\n",status));
		return status;
	}


	/*加载图像信息*/
	status=aos_init_graphics_info(&boot_params);
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Failed to get image information.\nError code:%u.\n",status));
		return status;
	}

	/*加载内核执行模块*/
	status=aos_load_bootstrap();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Kernel executable module file load failure.status=%u\n",status));
		return status;
	}

	/*扫描PCI设备*/
	status=aos_scan_pci_device();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Failed to scan PCI bus.\nError code:%u.\n",status));
		return status;
	}

	/*预分配配置空间*/
	status=aos_create_config();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Pre-allocation of configuration space failed.\nError code:%u.\n",status));
		return status;
	}

	/*获取内存表*/
	UINTN key;
	status=aos_set_memmap(&key);
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Failed to get memory table.\nError code:%u.\n",status));
		return status;
	}
	/*设置系统表*/
	status=aos_set_system_table();
	ASSERT(status==EFI_SUCCESS);
	
	/*复制启动参数表，使其脱离该程序成为固定内存*/
	VOID* params_region=aos_allocate_pool(sizeof(boot_params));
	ASSERT(params_region!=NULL);
	CopyMem(params_region,&boot_params,sizeof(boot_params));

	/*结束启动服务*/
	status=gBS->ExitBootServices(gImageHandle,key);
	ASSERT(status==EFI_SUCCESS);

	DEBUG((DEBUG_INFO,"===AOS UEFI TSL Module Summary===\n"));
	DEBUG((DEBUG_INFO,"boot_params_size=%lu\n",sizeof(boot_params)));
	DEBUG((DEBUG_INFO,".pool=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,pool)));
	DEBUG((DEBUG_INFO,".pool_length=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,pool_length)));
	DEBUG((DEBUG_INFO,".stack=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,stack)));
	DEBUG((DEBUG_INFO,".stack_length=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,stack_length)));
	DEBUG((DEBUG_INFO,".root_bridges=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,root_bridges)));
	DEBUG((DEBUG_INFO,".root_bridge_length=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,root_bridge_length)));
	DEBUG((DEBUG_INFO,".devices=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,devices)));
	DEBUG((DEBUG_INFO,".device_length=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,device_length)));
	DEBUG((DEBUG_INFO,".graphics_info=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,graphics_info)));
	DEBUG((DEBUG_INFO,".env=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,env)));
	DEBUG((DEBUG_INFO,".acpi=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,acpi)));
	DEBUG((DEBUG_INFO,".smbios=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,smbios)));
	DEBUG((DEBUG_INFO,".smbios3=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,smbios3)));
	DEBUG((DEBUG_INFO,".runtime=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,runtime)));
	DEBUG((DEBUG_INFO,".modules=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,modules)));
	DEBUG((DEBUG_INFO,".boot_device=%lu\n",aos_offset_of(AOS_BOOT_PARAMS,boot_device)));
	DEBUG((DEBUG_INFO,"===Jump aos.boot===\n"));

	SWITCH_STACK_ENTRY_POINT aos_boot_trampoline=NULL;
	/*跳转入口，可能会提前调用一个aos.boot.base模块作为最小c库*/
	aos_boot_trampoline=(SWITCH_STACK_ENTRY_POINT)(boot_params.modules[0].entry+boot_params.modules[0].base);
	aos_boot_trampoline(params_region,NULL);
	return EFI_SUCCESS;
}
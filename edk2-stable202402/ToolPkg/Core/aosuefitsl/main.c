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
	UINT32 i,j,c,h,v; /*绘制变量*/
	UINT32* offset;

	DEBUG((DEBUG_INFO,"===AOS UEFI TSL Module===\n"));

	/*关闭一下看门狗*/
	status=gBS->SetWatchdogTimer(0,0,0,NULL);
	if(status==EFI_DEVICE_ERROR)
	{
		/*出现看门狗硬件错误*/
		return status;
	}

	/*加载内核执行模块*/
	status=aos_load_bootstrap();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Kernel executable module file load failure.status=%u\n",status));
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

	/*扫描PCI设备*/
	status=aos_scan_pci_device();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Failed to scan PCI bus.\nError code:%u.\n",status));
		return status;
	}

	/*加载图像信息*/
	status=aos_init_graphics_info(&boot_params);
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"Failed to get image information.\nError code:%u.\n",status));
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

	/*尝试绘制染色板*/
	c=0;
	h=boot_params.graphics_info.scan_line_length;
	v=boot_params.graphics_info.vertical_resolution;
	offset=(UINT32*)boot_params.graphics_info.frame_buffer_base;
	for(j=0;j<v;j++)
	{
		for(i=0;i<h;i++)
		{
			*offset=c|0xFF000000;
			c++;
			if(c>=0x01000000)
			{
				c=0;
			}
			offset++;
		}
	}

	DEBUG((DEBUG_INFO,"sizeof(boot_params)=%lu\n",sizeof(boot_params)));

	/*计算一下总大小*/
	UINTN size_pci=0;
	VOID* table=(VOID*)boot_params.root_bridges;
	for(UINTN index=0;index<boot_params.root_bridge_length;index++)
	{
		UINTN t=sizeof(PCI_ROOT_BRIDGE_INFO)+((PCI_ROOT_BRIDGE_INFO*)table)->length*sizeof(PCI_RANGE);
		size_pci=size_pci+t;
		table=(VOID*)((UINTN)table+t);
	}
	size_pci=boot_params.device_length*sizeof(PCI_DEVICE_INFO)+size_pci;
	DEBUG((DEBUG_INFO,"size_pci=%lu,pages=%lu\n",size_pci,aos_size_to_pages(size_pci)));

	int (*aos_bootstrap_jump)(void* boot_params,void* unused)=NULL;
	aos_bootstrap_jump=(int (*)(void* boot_params,void* unused))(boot_params.modules[0].entry+boot_params.modules[0].base); 
	aos_bootstrap_jump(params_region,NULL);
	return EFI_SUCCESS;
}
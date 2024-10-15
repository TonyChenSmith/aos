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

	/*第一阶段*/
	/*关闭一下看门狗*/
	status=gBS->SetWatchdogTimer(0,0,0,NULL);
	if(status==EFI_DEVICE_ERROR)
	{
		/*出现看门狗硬件错误*/
		return status;
	}

	/*初始化启动参数*/
	status=aos_init_memory();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"1. status=%u\n",status));
		return status;
	}

	status=aos_scan_pci_device();
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"2. status=%u\n",status));
		return status;
	}

	status=aos_init_graphics_info(&boot_params);
	if(EFI_ERROR(status))
	{
		DEBUG((DEBUG_ERROR,"3. status=%u\n",status));
		return status;
	}

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
	/*暂停10s*/
	gBS->Stall(10000);

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
	return EFI_SUCCESS;
}
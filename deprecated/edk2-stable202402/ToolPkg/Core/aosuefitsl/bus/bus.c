/*
 * AOS UEFI Bootloader总线管理。
 * 主要用于扫描PCI总线映射内存空间。
 * 2024-09-22创建。
 * 
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "bus/aosbus.h"

/*局部量*/

/*长度，0为根桥数目，1为设备数目*/
static UINTN lengths[2];

/*句柄，0为根桥，1为设备*/
static EFI_HANDLE* handles[2];

/*
 * 扫描PCI总线。将把扫描信息进行统计和记录，特别是地址
 * @retval EFI_SUCCESS 执行成功。
 * @retval EFI_OUT_OF_RESOURCES 内存资源不足。
 */
EFI_STATUS
EFIAPI
aos_scan_pci_device(
	VOID
)
{
	EFI_STATUS status; /*状态*/

	/*第一步，找到所有设备*/
	lengths[0]=0;
	status=gBS->LocateHandle(
		ByProtocol,
		&gEfiPciRootBridgeIoProtocolGuid,
		NULL,
		&lengths[0],
		handles[0]
	);
	if(status==EFI_BUFFER_TOO_SMALL)
	{
		VOID* buffer;
		status=gBS->AllocatePool(EfiBootServicesData,lengths[0],&buffer);
		if(EFI_ERROR(status))
		{
			return status;
		}
		handles[0]=(EFI_HANDLE*)buffer;
		status=gBS->LocateHandle(
			ByProtocol,
			&gEfiPciRootBridgeIoProtocolGuid,
			NULL,
			&lengths[0],
			handles[0]
		);
	}
	if(EFI_ERROR(status))
	{
		return status;
	}
	lengths[0]=lengths[0]/sizeof(EFI_HANDLE);
	lengths[1]=0;
	status=gBS->LocateHandle(
		ByProtocol,
		&gEfiPciIoProtocolGuid,
		NULL,
		&lengths[1],
		handles[1]
	);
	if(status==EFI_BUFFER_TOO_SMALL)
	{
		VOID* buffer;
		status=gBS->AllocatePool(EfiBootServicesData,lengths[1],&buffer);
		if(EFI_ERROR(status))
		{
			return status;
		}
		handles[1]=(EFI_HANDLE*)buffer;
		status=gBS->LocateHandle(
			ByProtocol,
			&gEfiPciIoProtocolGuid,
			NULL,
			&lengths[1],
			handles[1]
		);
	}
	if(EFI_ERROR(status))
	{
		return status;
	}
	lengths[1]=lengths[1]/sizeof(EFI_HANDLE);

	boot_params.root_bridge_length=lengths[0];
	boot_params.root_bridges=NULL;

	DEBUG((DEBUG_INFO,"==AOS PCI Root Bridge==\n"));

	/*第二步，遍历根桥获取信息*/
	for(UINTN index=0;index<lengths[0];index++)
	{
		EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* rbprotocol;
		VOID* addr_base;
		PCI_RANGE* range;
		PCI_ROOT_BRIDGE_INFO* info=(PCI_ROOT_BRIDGE_INFO*)aos_allocate_pool(sizeof(PCI_ROOT_BRIDGE_INFO));
		if(info==NULL)
		{
			/*资源不足*/
			return EFI_OUT_OF_RESOURCES;
		}
		status=gBS->HandleProtocol(handles[0][index],&gEfiPciRootBridgeIoProtocolGuid,(VOID*)&rbprotocol);
		if(EFI_ERROR(status))
		{
			return status;
		}
		info->segment=rbprotocol->SegmentNumber;
		status=rbprotocol->GetAttributes(rbprotocol,&info->supports,&info->attributes);
		if(EFI_ERROR(status))
		{
			return status;
		}

		DEBUG((DEBUG_INFO,"PCI[%lu]:S:%lX,A:%lX\n",info->segment,info->supports,info->attributes));

		status=rbprotocol->Configuration(rbprotocol,&addr_base);
		if(status==EFI_UNSUPPORTED)
		{
			/*没有描述符，应该假设0~255*/
			range=(PCI_RANGE*)aos_allocate_pool(sizeof(PCI_RANGE));
			if(range==NULL)
			{
				return EFI_OUT_OF_RESOURCES;
			}
			info->length=1;
			range->mix=0;
			range->max=255;
			DEBUG((DEBUG_INFO,"    Range[%lu]:%lu~%lu\n",index,range->mix,range->max));
		}
		else
		{
			BOOLEAN next=TRUE;
			PCI_QWORD_TABLE* table=(PCI_QWORD_TABLE*)addr_base;
			while(next)
			{
				switch(table->qword_desciptor)
				{
					case 0x8A:
						{
							/*地址表*/
							if(!(table->resource_type==2))
							{
								/*非总线区段，跳过*/
								table=(PCI_QWORD_TABLE*)((VOID*)((UINTN)table+table->qword_length+3));
								break;
							}
							range=(PCI_RANGE*)aos_allocate_pool(sizeof(PCI_RANGE));
							if(range==NULL)
							{
								return EFI_OUT_OF_RESOURCES;
							}
							info->length++;
							range->mix=table->address_range_minimum;
							range->max=table->address_range_maximum;

							DEBUG((DEBUG_INFO,"    Range[%lu]:%lu~%lu\n",index,range->mix,range->max));

							table=(PCI_QWORD_TABLE*)((VOID*)((UINTN)table+table->qword_length+3));
							break;
						}
					case 0x79:
						{
							/*结尾*/
							next=FALSE;
							break;
						}
					default:
						{
							/*其他表，跳过*/
							table=(PCI_QWORD_TABLE*)((VOID*)((UINTN)table+table->qword_length+3));
							break;
						}
				}
			}
		}

		DEBUG((DEBUG_INFO,"    Range:%lu\n",info->length));

		if(boot_params.root_bridges==NULL)
		{
			boot_params.root_bridges=info;
		}
	}

	DEBUG((DEBUG_INFO,"==AOS PCI Device==\n"));

	boot_params.device_length=lengths[1];
	boot_params.devices=(PCI_DEVICE_INFO*)aos_allocate_pool(lengths[1]*sizeof(PCI_DEVICE_INFO));
	if(boot_params.devices==NULL)
	{
		return EFI_OUT_OF_RESOURCES;
	}
	PCI_DEVICE_INFO* devices=boot_params.devices;

	/*第三步，遍历PCI设备*/
	for(UINTN index=0;index<lengths[1];index++)
	{
		EFI_PCI_IO_PROTOCOL* iodev;
		status=gBS->HandleProtocol(handles[1][index],&gEfiPciIoProtocolGuid,(VOID*)&iodev);
		if(EFI_ERROR(status))
		{
			DEBUG((DEBUG_ERROR,"21. status=%u\n",status));
			return status;
		}
		status=iodev->GetLocation(iodev,&devices[index].segment,&devices[index].bus,&devices[index].device,&devices[index].function);
		if(EFI_ERROR(status))
		{
			DEBUG((DEBUG_ERROR,"22. status=%u\n",status));
			return status;
		}
		status=iodev->Attributes(iodev,EfiPciIoAttributeOperationSupported,0,&devices[index].supports);
		if(EFI_ERROR(status))
		{
			DEBUG((DEBUG_ERROR,"23. status=%u\n",status));
			return status;
		}
		status=iodev->Attributes(iodev,EfiPciIoAttributeOperationGet,0,&devices[index].attributes);
		if(EFI_ERROR(status))
		{
			DEBUG((DEBUG_ERROR,"24. status=%u\n",status));
			return status;
		}
		DEBUG((DEBUG_INFO,"PCI[%lu:%lu:%lu:%lu]:S:%lX,A:%lX\n",devices[index].segment,devices[index].bus,devices[index].device,devices[index].function,devices[index].supports,devices[index].attributes));
		for(UINT8 barN=0;barN<6;barN++)
		{
			PCI_QWORD_TABLE* table;
			VOID* pointer;
			status=iodev->GetBarAttributes(iodev,barN,&devices[index].bars[barN].supports,&pointer);
			if(status==EFI_UNSUPPORTED)
			{
				devices[index].bars[barN].resource=4;
			}
			else if(EFI_ERROR(status))
			{
				return status;
			}
			else
			{
				table=(PCI_QWORD_TABLE*)pointer;
				BOOLEAN next_table;
				do
				{
					next_table=FALSE;
					if(table->qword_desciptor!=0x8A)
					{
						next_table=TRUE;
						table=(PCI_QWORD_TABLE*)((VOID*)((UINTN)table+table->qword_length+3));
						continue;
					}
					else
					{
						devices[index].bars[barN].resource=table->resource_type;
						devices[index].bars[barN].general=table->general_flags;
						devices[index].bars[barN].type_specific=table->type_specific_flags;
						devices[index].bars[barN].granularity=table->address_space_granularity;
						devices[index].bars[barN].min=table->address_range_minimum;
						devices[index].bars[barN].max=table->address_range_maximum;
						devices[index].bars[barN].length=table->address_length;
						devices[index].bars[barN].offset=table->address_translation_offset;
					}
				}while(next_table);
				gBS->FreePool(pointer);
			}
			DEBUG((DEBUG_INFO,"    Bar[%u]:T:%u\n",barN,devices[index].bars[barN].resource));
			if(devices[index].bars[barN].resource!=4)
			{
				DEBUG((DEBUG_INFO,"    Bar[%u]:G=%X,T=%X,g=%lu\n",barN,devices[index].bars[barN].general,devices[index].bars[barN].type_specific,devices[index].bars[barN].granularity));
				DEBUG((DEBUG_INFO,"        min=0x%lX,max=0x%lX,length=0x%lX,offset=0x%lX\n",devices[index].bars[barN].min,devices[index].bars[barN].max,devices[index].bars[barN].length,devices[index].bars[barN].offset));
			}
		}
	}
	gBS->FreePool((VOID*)handles[0]);
	gBS->FreePool((VOID*)handles[1]);
	return EFI_SUCCESS;
}
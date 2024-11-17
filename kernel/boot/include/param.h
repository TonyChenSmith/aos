/*
 * aos.boot输入参数类型定义。
 * @date 2024-10-23
 */
#ifndef __AOS_BOOT_PARAM_H__
#define __AOS_BOOT_PARAM_H__

#include "type.h"
#include "fw\efi.h"

/*启动模块个数*/
#define BOOT_MODULE_COUNT 3

/*PCI总线范围*/
typedef struct _boot_pci_bus
{
	uintn mix; /*最小值*/
	uintn max; /*最大值*/
} boot_pci_bus;

/*PCI根桥信息*/
typedef struct _boot_pci_root_info
{
	uintn segment;		  /*段号*/
	uint64 supports;	  /*支持属性*/
	uint64 attributes;	  /*属性，具体定义查看UEFI规范相关章节。*/
	uintn length;		  /*总线范围数组长度*/
	boot_pci_bus buses[]; /*总线范围*/
} boot_pci_root_info;

/*PCI范围属性*/
typedef struct _boot_pci_bar
{
	uint64 supports;	 /*支持属性*/
	uint8 resource;		 /*资源类型,4代表不存在该BAR*/
	uint8 general;		 /*通用标志*/
	uint8 type_specific; /*类型特有标志*/
	uintn granularity;	 /*粒度*/
	uintn min;			 /*最小值*/
	uintn max;			 /*最大值*/
	uintn offset;		 /*偏移*/
	uintn length;		 /*长度*/ 
} boot_pci_bar;

/*PCI设备信息*/
typedef struct _boot_pci_info
{
	uintn segment;		  /*段号*/
	uintn bus;			  /*总线*/
	uintn device;		  /*设备*/
	uintn function;		  /*功能*/
	uint64 supports;	  /*支持属性*/
	uint64 attributes;	  /*属性，具体定义查看UEFI规范相关章节。*/
	boot_pci_bar bars[6]; /*BAR信息*/
} boot_pci_info;

/*图像信息*/
typedef struct _boot_graphics_info
{
	uint32 horizontal_resolution; /*水平分辨率，x轴范围*/
	uint32 vertical_resolution;	  /*竖直分辨率，y轴范围*/
	uint64 frame_buffer_base;	  /*帧缓存区基址*/
	uint64 frame_buffer_size;	  /*帧缓存区大小*/
	uint32 red_mask;			  /*红区位*/
	uint32 green_mask;			  /*绿区位*/
	uint32 blue_mask;			  /*蓝区位*/
	uint32 reserved_mask;		  /*保留区位*/
	uint32 scan_line_length;	  /*扫描线长度。小于等下水平分辨率*/
} boot_graphics_info;

/*CPU信息*/
typedef struct _boot_cpu_info
{
	uint32 max_processors; /*CPU拥有的逻辑处理器核心数*/
	bool level5;		   /*是否支持五级页*/
	bool page1gb;		   /*1GB大页*/
	bool pge;			   /*全局页*/
	bool erms;			   /*ERMS支持*/
} boot_cpu_info;

/*描述符寄存器内容*/
#pragma pack(1)
typedef struct _dtr_info
{
	uint16 limit; /*范围*/
	uintn base;	  /*基址*/
} dtr_info;
#pragma pack()

/*环境信息*/
typedef struct _boot_env_info
{
	boot_cpu_info cpu_info;		   /*CPU信息*/
	efi_memory_descriptor* memmap; /*内存图*/
	uintn entry_size;			   /*单项的大小*/
	uintn memmap_length; 		   /*内存图长度，字节计数*/
} boot_env_info;

/*引导程序信息*/
typedef struct _boot_code_info
{
	uintn base;	 /*程序基址，物理地址*/
	uintn entry; /*入口偏移*/
	uintn pages; /*页面数目*/
} boot_code_info;

/*启动参数*/
typedef struct _boot_params
{
	void* pool; 							   /*内存池基址*/
	uintn pool_length; 						   /*内存池长度*/
	void* stack; 							   /*栈基址*/
	uintn stack_length; 					   /*栈长度*/

	boot_pci_root_info* root_bridges; 		   /*根桥信息数组*/
	uintn root_bridge_length; 				   /*根桥数目*/
	boot_pci_info* devices; 				   /*设备信息数组*/
	uintn device_length; 					   /*设备数目*/

	boot_graphics_info graphics_info; 		   /*图像信息数据结构*/

	boot_env_info env; 						   /*环境信息*/

	void* acpi;								   /*ACPI表*/
	void* smbios;							   /*SMBIOS表*/
	void* smbios3;							   /*SMBIOS3表*/
	efi_runtime_services* runtime;			   /*UEFI运行服务*/

	void* boot_device;						   /*启动设备路径*/
	boot_code_info modules[BOOT_MODULE_COUNT]; /*启动模块信息*/
} boot_params;

#endif /*__AOS_BOOT_PARAM_H__*/
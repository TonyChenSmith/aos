/*
 * AOS UEFI固件函数表。
 * @date 2024-10-24
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_FW_EFI_H__
#define __AOS_FW_EFI_H__

#include "basic_type.h"

/*EFI返回类型*/
typedef uintn efi_status;

/*EFI物理描述符*/
typedef struct _efi_memory_descriptor
{
	uint32 type;		  /*类型*/
	uintn physical_start; /*物理起始*/
	uintn virtual_start;  /*虚拟起始*/
	uint64 pages;		  /*页数*/
	uint64 attribute;	  /*页面属性*/
} efi_memory_descriptor;

/*EFI表头*/
typedef struct _efi_table_header
{
	uint64 signature;	/*表签名*/
	uint32 revision;	/*版本*/
	uint32 header_size; /*表大小*/
	uint32 crc32;		/*CRC32校验*/
	uint32 reserved;	/*保留*/
} efi_table_header;

/*EFI运行服务表*/
typedef struct _efi_runtime_services
{
	efi_table_header hdr;				 /*表头*/
  	void* get_time;						 /*获得时间*/
	void* set_time;						 /*设置时间*/
	void* get_wakeup_time;				 /*未使用*/
	void* set_wakeup_time;				 /*未使用*/
  	void* set_virtual_address_map;		 /*设置虚拟地址映射*/
	void* convert_pointer;				 /*指针变换*/
	void* get_variable;					 /*获得变量*/
	void* get_next_variable_name;		 /*获得下一个变量名*/
	void* set_variable;					 /*设置变量*/
	void* get_next_high_monotonic_count; /*未使用*/
	void* reset_system;					 /*重置系统*/
	void* update_capsule;				 /*未使用*/
	void* query_capsule_capabilities;	 /*未使用*/
	void* query_variable_info;			 /*获取变量信息*/
} efi_runtime_services;

/*EFI内存类型*/
typedef enum _efi_memory_type
{
	EFI_RESERVED_MEMORY_TYPE,		 /*保留*/
	EFI_LOADER_CODE,				 /*引导器代码*/
	EFI_LOADER_DATA,				 /*引导器数据*/
	EFI_BOOT_SERVICES_CODE,			 /*启动时代码*/
	EFI_BOOT_SERVICES_DATA,			 /*启动时数据*/
	EFI_RUNTIME_SERVICES_CODE,		 /*运行时代码*/
	EFI_RUNTIME_SERVICES_DATA,		 /*运行时数据*/
	EFI_CONVENTIONAL_MEMORY,		 /*自由内存*/
	EFI_UNUSABLE_MEMORY,			 /*不使用*/
	EFI_ACPI_RECLAIM_MEMORY,		 /*ACPI表*/
	EFI_ACPI_MEMORY_NVS,			 /*ACPI NVS*/
	EFI_MEMORY_MAPPED_IO,			 /*MMIO*/
	EFI_MEMORY_MAPPED_IO_PORT_SPACE, /*MMIO端口空间*/
	EFI_PAL_CODE,					 /*处理器固件保留*/
	EFI_PERSISTENT_MEMORY,			 /*非易失字节访问*/
	EFI_UNACCEPTED_MEMORY_TYPE,		 /*未接受*/
	EFI_MAX_MEMORY_TYPE				 /*最大内存类型*/
} efi_memory_type;

/*EFI重置类型*/
typedef enum _efi_reset_type
{
	EFI_RESET_COLD,				/*冷重启*/
	EFI_RESET_WARM,				/*暖重启*/
	EFI_RESET_SHUTDOWN,			/*关机*/
	EFI_RESET_PLATFORM_SPECIFIC	/*平台自定义*/
} efi_reset_type;

/*EFI成功*/
#define EFI_SUCCESS 0

/*EFI资源不足*/
#define EFI_OUT_OF_RESOURCES 9

#endif /*__AOS_FW_EFI_H__*/
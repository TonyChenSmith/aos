/*
 * aos内核UEFI定义。
 * @date 2024-10-24
 */
#ifndef __AOS_BASE_EFI_H__
#define __AOS_BASE_EFI_H__

#include "type.h"

/*EFI返回类型*/
typedef uintn efi_status;

/*EFI物理描述符*/
typedef struct _efi_memory_descriptor
{
	uint32 type; /*类型*/
	uintn physical_start; /*物理起始*/
	uintn virtual_start; /*虚拟起始*/
	uint64 pages; /*页数*/
	uint64 attribute; /*页面属性*/
} efi_memory_descriptor;

/*EFI表头*/
typedef struct _efi_table_header
{
	uint64 signature; /*表签名*/
	uint32 revision; /*版本*/
	uint32 header_size; /*表大小*/
	uint32 crc32; /*CRC32校验*/
	uint32 reserved; /*保留*/
} efi_table_header;

/*EFI运行服务表*/
typedef struct _efi_runtime_services
{
	efi_table_header hdr; /*表头*/
  	void* get_time; /*获得时间*/
	void* set_time; /*设置时间*/
	void* get_wakeup_time;
	void* set_wakeup_time;
  	void* set_virtual_address_map;
	void* convert_pointer;
	void* get_variable;
	void* get_next_variable_name;
	void* set_variable;
	void* get_next_high_monotonic_count;
	void* reset_system;
	void* update_capsule;
	void* query_capsule_capabilities;
	void* query_variable_info;
} efi_runtime_services;

#endif /*__AOS_BASE_EFI_H__*/
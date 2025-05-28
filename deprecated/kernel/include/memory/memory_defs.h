/*
 * 内存定义。
 * @date 2024-12-12
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_MEMORY_MEMORY_DEFS_H__
#define __AOS_MEMORY_MEMORY_DEFS_H__

/*内存类型*/
typedef enum _memory_type
{
	AOS_RESERVED,		/*保留*/
	AOS_AVAILABLE,		/*可用*/
	AOS_ACPI_TABLE,		/*ACPI表*/
	AOS_ACPI_NVS,		/*ACPI固件*/
	AOS_FIRMWARE_CODE,	/*固件代码*/
	AOS_FIRMWARE_DATA,	/*固件数据*/
	AOS_KERNEL_CODE,	/*内核代码*/
	AOS_KERNEL_DATA,	/*内核数据*/
	AOS_USER_CODE,		/*用户代码*/
	AOS_USER_DATA,		/*用户数据*/
	AOS_MMIO,			/*内存端口映射*/
	AOS_LOWEST_MEMORY,	/*最低1MB区内存*/
	AOS_LOADER_DATA,	/*UEFI核创建数据*/
	AOS_MAX_MEMORY_TYPE /*最大内存类型*/
} memory_type;

/*内存申请模式*/
typedef enum _malloc_mode
{
	MALLOC_MIN,		/*最小下限*/
	MALLOC_MAX,		/*最大上限*/
	MALLOC_RANGE,	/*范围申请*/
	MALLOC_ANY,		/*任意*/
	MALLOC_MAX_MODE	/*最大模式*/
} malloc_mode;

#endif /*__AOS_MEMORY_MEMORY_DEFS_H__*/
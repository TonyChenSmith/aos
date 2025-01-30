/*
 * 启动核参数定义。
 * @date 2024-10-23
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_PARAMS_H__
#define __AOS_BOOT_PARAMS_H__

#include "fw/efi.h"

/*启动核内存块个数*/
#define BOOT_BLOCK_COUNT 3

/*固定内存块*/
#define BOOT_FIXED_BLOCK 0

/*内存图内存块*/
#define BOOT_MEMORY_MAP_BLOCK 1

/*物理内存管理系统内存块*/
#define BOOT_PMMS_BLOCK 2

/*启动核模块个数*/
#define BOOT_MODULE_COUNT 3

/*启动核初始化模块*/
#define BOOT_INIT_MODULE 0

/*启动核内存模块*/
#define BOOT_MEMORY_MODULE 1

/*启动核工具模块*/
#define BOOT_UTIL_MODULE 2

/*模块顺序为：init,memory,util*/

/*内存块范围*/
typedef struct _boot_block
{
	uintn base;	 /*内存基址*/
	uintn pages; /*内存页数*/
} boot_block;

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
	uint32 scan_line_length;	  /*扫描线长度。小于等于水平分辨率*/
} boot_graphics_info;

/*环境信息*/
typedef struct _boot_env_info
{
	uint32 processors;			   /*已知逻辑处理器核心数*/
	uint32 enable_processors;	   /*可用逻辑处理器核心数*/
	efi_memory_descriptor* memmap; /*内存映射*/
	uintn entry_size;			   /*单项内存条目大小*/
	uintn memmap_length; 		   /*内存映射长度，字节计数*/
} boot_env_info;

/*启动核程序信息*/
typedef struct _boot_code_info
{
	uintn base;	 /*程序基址，物理地址*/
	uintn entry; /*入口偏移*/
	uintn pages; /*页面数目*/
} boot_code_info;

/*启动核参数*/
typedef struct _boot_params
{
	boot_block block[BOOT_BLOCK_COUNT];		   /*内存块信息*/

	boot_graphics_info graphics_info; 		   /*图像信息数据结构*/

	boot_env_info env; 						   /*环境信息*/

	void* acpi;								   /*ACPI表*/
	void* smbios;							   /*SMBIOS表*/
	void* smbios3;							   /*SMBIOS3表*/
	efi_runtime_services* runtime;			   /*UEFI运行服务*/

	void* boot_device;						   /*启动设备路径*/
	boot_code_info modules[BOOT_MODULE_COUNT]; /*启动核模块信息*/
} boot_params;

#endif /*__AOS_BOOT_PARAMS_H__*/
/*
 * 初始化模块定义。定义了启动核初始化流程。
 * @date 2025-01-18
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_INIT_H__
#define __AOS_BOOT_INIT_H__

#include "module/util.h"
#include "module/memory.h"

/*启动核函数表*/
struct _boot_function_table
{
	boot_util_function* util;	  /*工具模块函数表*/
	boot_memory_function* memory; /*内存模块函数表*/
};

#endif /*__AOS_BOOT_INIT_H__*/
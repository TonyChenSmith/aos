/**
 * 内核内存管理模块初始化。
 * @date 2026-03-11
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_MEMORY_INIT_H__
#define __AOS_KERNEL_MEMORY_INIT_H__

#include <init/params.h>

/**
 * 通过启动参数初始化内存管理模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_memory_init(aos_boot_params* params);

#endif /*__AOS_KERNEL_MEMORY_INIT_H__*/
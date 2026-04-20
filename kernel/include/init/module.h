/**
 * 内核模块初始化函数。
 * @date 2026-04-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_INIT_MODULE_H__
#define __AOS_KERNEL_INIT_MODULE_H__

#include "params.h"

/**
 * 通过启动参数初始化内存管理模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_memory_init(aos_boot_params* params);

/**
 * 通过启动参数初始化固件模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_firmware_init(aos_boot_params *params);

#endif /*__AOS_KERNEL_INIT_MODULE_H__*/
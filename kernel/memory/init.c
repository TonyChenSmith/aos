/**
 * 内核内存管理模块初始化。
 * @date 2026-03-12
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>

/**
 * 内核内存池参考基址。
 */
const uintn KERNEL_POOL_BASE=-5*SIZE_512GB;

/**
 * 通过启动参数初始化内存管理模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_memory_init(aos_boot_params* params)
{
    
}
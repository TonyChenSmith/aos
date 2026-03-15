/**
 * 内核内存管理模块内部声明和定义。
 * @date 2026-03-14
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_MEMORY_MEMORY_INTERNAL_H__
#define __AOS_KERNEL_MEMORY_MEMORY_INTERNAL_H__

#include <support/type.h>

/**
 * 初始化内核内存池。
 * 
 * @param pool  内核内存池基址。
 * @param pages 内存块页数。
 * 
 * @return 成功初始化返回真。
 */
bool pool_init(void* pool,uintn pages);

#endif /*__AOS_KERNEL_MEMORY_MEMORY_INTERNAL_H__*/
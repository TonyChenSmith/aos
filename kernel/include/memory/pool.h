/**
 * 内核内存池分配器。
 * @date 2026-03-14
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_MEMORY_POOL_H__
#define __AOS_KERNEL_MEMORY_POOL_H__

#include <support/type.h>

/**
 * 申请一块内核内存池的内存。
 * 
 * @param size 申请大小。
 * 
 * @return 成功申请返回一个非空指针。
 */
void* pool_alloc(uintn size);

/**
 * 释放一块内核内存池的内存。
 * 
 * @param ptr 申请内存指针基址。
 * 
 * @return 无返回值。
 */
void pool_free(void* ptr);

#endif /*__AOS_KERNEL_MEMORY_POOL_H__*/
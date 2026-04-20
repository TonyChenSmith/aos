/**
 * 内核支持模块级别同步原语。
 * @date 2026-03-29
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_SYNC_H__
#define __AOS_KERNEL_SUPPORT_SYNC_H__

#include "atomic.h"

/**
 * 自旋锁。
 */
typedef struct _spinlock
{
    atomic_flag flag; /*标志。*/
} spinlock;

/**
 * 自旋锁初始化。
 * 
 * @param lock 自旋锁。
 * 
 * @return 无返回值。
 */
void spinlock_init(spinlock* lock);

/**
 * 自旋锁阻塞加锁。
 * 
 * @param lock 自旋锁。
 * 
 * @return 无返回值。
 */
void spinlock_lock(spinlock* lock);

/**
 * 自旋锁尝试加锁。
 * 
 * @param lock 自旋锁。
 * 
 * @return 成功上锁返回真。
 */
bool spinlock_try_lock(spinlock* lock);

/**
 * 自旋锁解锁。
 * 
 * @param lock 自旋锁。
 * 
 * @return 成功上锁返回真。
 */
void spinlock_unlock(spinlock* lock);

#endif /*__AOS_KERNEL_SUPPORT_SYNC_H__*/
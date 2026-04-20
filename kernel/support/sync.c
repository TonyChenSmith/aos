/**
 * 内核支持模块级别同步原语。
 * @date 2026-03-29
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/control.h>
#include <support/sync.h>

/**
 * 自旋锁初始化。
 * 
 * @param lock 自旋锁。
 * 
 * @return 无返回值。
 */
void spinlock_init(spinlock* lock)
{
    if(lock!=null)
    {
        atomic_init(&lock->flag,ATOMIC_FLAG_INIT);
    }
}

/**
 * 自旋锁阻塞加锁。
 * 
 * @param lock 自旋锁。
 * 
 * @return 无返回值。
 */
void spinlock_lock(spinlock* lock)
{
    if(lock!=null)
    {
        while(atomic_flag_test_and_set_explicit(&lock->flag,MEMORY_ORDER_ACQUIRE))
        {
            cpu_pause();
        }
    }
    else
    {
        /*锁指针为空必须终止CPU以保证数据安全，此时应该重启计算机了*/
        while(true)
        {
            cpu_halt();
        }
    }
}

/**
 * 自旋锁尝试加锁。
 * 
 * @param lock 自旋锁。
 * 
 * @return 成功上锁返回真。
 */
bool spinlock_try_lock(spinlock* lock)
{
    return lock!=null&&!atomic_flag_test_and_set_explicit(&lock->flag,MEMORY_ORDER_ACQUIRE);
}

/**
 * 自旋锁解锁。
 * 
 * @param lock 自旋锁。
 * 
 * @return 成功上锁返回真。
 */
void spinlock_unlock(spinlock* lock)
{
    if(lock!=null)
    {
        atomic_flag_clear_explicit(&lock->flag,MEMORY_ORDER_RELEASE);
    }
}
/**
 * 内核CPU控制函数。
 * @date 2026-04-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_CONTROL_H__
#define __AOS_KERNEL_SUPPORT_CONTROL_H__

#include "const.h"

/**
 * 禁用中断。
 * 
 * @return 无返回值。
 */
static inline void disable_interrupts(void)
{
    __asm__ volatile("cli":::"memory");
}
/**
 * 启用中断。
 * 
 * @return 无返回值。
 */
static inline void enable_interrupts(void)
{
    __asm__ volatile("sti":::"memory");
}

/**
 * 让CPU暂停一下。优化忙等待性能。
 * 
 * @return 无返回值。
 */
static inline void cpu_pause(void)
{
    __asm__ volatile("pause":::"memory");
}

/**
 * 让CPU终止，直到一个中断发生。
 * 
 * @return 无返回值。
 */
static inline void cpu_halt(void)
{
    __asm__ volatile("hlt":::"memory");
}

#endif /*__AOS_KERNEL_SUPPORT_CONTROL_H__*/
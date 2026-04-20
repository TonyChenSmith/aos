/**
 * 内核内存屏障。
 * @date 2026-04-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_BARRIER_H__
#define __AOS_KERNEL_SUPPORT_BARRIER_H__

/**
 * 执行存储屏障。
 */
#define store_barrier() __asm__ volatile("sfence":::"memory")

/**
 * 执行加载屏障。
 */
#define load_barrier() __asm__ volatile("lfence":::"memory")

/**
 * 执行内存屏障。
 */
#define memory_barrier() __asm__ volatile("mfence":::"memory")

/**
 * 执行编译器屏障。
 */
#define compiler_barrier() __asm__ volatile("":::"memory")

#endif /*__AOS_KERNEL_SUPPORT_BARRIER_H__*/
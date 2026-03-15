/**
 * 内核汇编内联函数。
 * @date 2026-03-09
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_ASM_H__
#define __AOS_KERNEL_SUPPORT_ASM_H__

#include "type.h"

/**
 * 执行存储屏障。
 * 
 * @return 无返回值。
 */
static inline void store_fence()
{
    __asm__ volatile("sfence":::"memory");
}

/**
 * 执行加载屏障。
 * 
 * @return 无返回值。
 */
static inline void load_fence()
{
    __asm__ volatile("lfence":::"memory");
}

/**
 * 执行内存屏障。
 * 
 * @return 无返回值。
 */
static inline void memory_fence()
{
    __asm__ volatile("mfence":::"memory");
}

/**
 * 读取x86平台64位MSR。
 * 
 * @param index 寄存器地址。
 * 
 * @return 读取寄存器的值。
 */
static inline uint64 read_msr(uint32 index)
{
    uint32 low,high;
    __asm__ volatile("rdmsr":"=a"(low),"=d"(high):"c"(index));
    return ((uint64)high<<32)|low;
}

/**
 * 写入x86平台64位MSR。
 * 
 * @param index 寄存器地址。
 * @param value 待写入寄存器的值。
 * 
 * @return 无返回值。
 */
static inline void write_msr(uint32 index,uint64 value)
{
    __asm__ volatile("wrmsr"::"a"(value&0xFFFFFFFF),"d"(value>>32),"c"(index));
}

#endif /*__AOS_KERNEL_SUPPORT_ASM_H__*/
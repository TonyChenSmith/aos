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
static inline void x86_disable_interrupt(void)
{
    __asm__ volatile("cli":::"memory");
}
/**
 * 启用中断。
 * 
 * @return 无返回值。
 */
static inline void x86_enable_interrupt(void)
{
    __asm__ volatile("sti":::"memory");
}

/**
 * 让CPU暂停一下。优化忙等待性能。
 * 
 * @return 无返回值。
 */
static inline void x86_cpu_pause(void)
{
    __asm__ volatile("pause":::"memory");
}

/**
 * 让CPU终止，直到一个中断发生。
 * 
 * @return 无返回值。
 */
static inline void x86_cpu_halt(void)
{
    __asm__ volatile("hlt":::"memory");
}

/**
 * 初始化x87 FPU状态。
 * 
 * @return 无返回值。
 */
static inline void x86_fpu_init(void)
{
    __asm__ volatile("fninit");
}

/**
 * 保存FPU、SSE状态到保存区域。
 * 
 * @param region 保存区域。
 * 
 * @return 无返回值。
 */
static inline void x86_save_fpu_sse(void* region)
{
    __asm__ volatile("fxsave64 (%0)"::"r"(region):"memory");
}

/**
 * 从保存区域恢复FPU、SSE状态。
 * 
 * @param region 保存区域。
 * 
 * @return 无返回值。
 */
static inline void x86_restore_fpu_sse(const void* region)
{
    __asm__ volatile("fxrstor64 (%0)"::"r"(region):"memory");
}

/**
 * 保存扩展状态到保存区域。
 * 
 * @param region 保存区域。
 * @param mask   状态组件掩码。
 * 
 * @return 无返回值。
 */
static inline void x86_save_xstate(void* region,uint64 mask)
{
    uint32 low=(uint32)(mask&UINT32_MAX);
    uint32 high=(uint32)(mask>>32);
    __asm__ volatile("xsave64 (%0)"::"r"(region),"a"(low),"d"(high):"memory");
}

/**
 * 保存优化的扩展状态到保存区域。
 * 
 * @param region 保存区域。
 * @param mask   状态组件掩码。
 * 
 * @return 无返回值。
 */
static inline void x86_save_optimized_xstate(void* region,uint64 mask)
{
    uint32 low=(uint32)(mask&UINT32_MAX);
    uint32 high=(uint32)(mask>>32);
    __asm__ volatile("xsaveopt64 (%0)"::"r"(region),"a"(low),"d"(high):"memory");
}

/**
 * 保存压缩的扩展状态到保存区域。
 * 
 * @param region 保存区域。
 * @param mask   状态组件掩码。
 * 
 * @return 无返回值。
 */
static inline void x86_save_compacted_xstate(void* region,uint64 mask)
{
    uint32 low=(uint32)(mask&UINT32_MAX);
    uint32 high=(uint32)(mask>>32);
    __asm__ volatile("xsavec64 (%0)"::"r"(region),"a"(low),"d"(high):"memory");
}

/**
 * 特权级保存扩展状态到保存区域。
 * 
 * @param region 保存区域。
 * @param mask   状态组件掩码。
 * 
 * @return 无返回值。
 */
static inline void x86_supervisor_save_xstate(void* region,uint64 mask)
{
    uint32 low=(uint32)(mask&UINT32_MAX);
    uint32 high=(uint32)(mask>>32);
    __asm__ volatile("xsaves64 (%0)"::"r"(region),"a"(low),"d"(high):"memory");
}

/**
 * 从保存区域恢复扩展状态。
 * 
 * @param region 保存区域。
 * @param mask   状态组件掩码。
 * 
 * @return 无返回值。
 */
static inline void x86_restore_xstate(const void* region,uint64 mask)
{
    uint32 low=(uint32)(mask&UINT32_MAX);
    uint32 high=(uint32)(mask>>32);
    __asm__ volatile("xrstor64 (%0)"::"r"(region),"a"(low),"d"(high):"memory");
}

/**
 * 特权级从保存区域恢复扩展状态。
 * 
 * @param region 保存区域。
 * @param mask   状态组件掩码。
 * 
 * @return 无返回值。
 */
static inline void x86_supervisor_restore_xstate(const void* region,uint64 mask)
{
    uint32 low=(uint32)(mask&UINT32_MAX);
    uint32 high=(uint32)(mask>>32);
    __asm__ volatile("xrstors64 (%0)"::"r"(region),"a"(low),"d"(high):"memory");
}

#endif /*__AOS_KERNEL_SUPPORT_CONTROL_H__*/
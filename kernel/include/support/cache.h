/**
 * 内核CPU缓存管理函数。
 * @date 2026-04-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_CACHE_H__
#define __AOS_KERNEL_SUPPORT_CACHE_H__

#include "const.h"

/**
 * 刷新单个TLB条目。
 * 
 * @param vaddr 需要刷新地址。
 * 
 * @return 无返回值。
 */
static inline void x86_flush_single_tlb(uintn vaddr)
{
    __asm__ volatile("invlpg (%0)"::"r"(vaddr):"memory");
}

/**
 * 刷新全部TLB条目。
 * 
 * @return 无返回值。
 */
static inline void x86_flush_all_tlbs(void)
{
    uintn cr3;
    __asm__ volatile("mov %%cr3,%0\n\t"
                     "mov %0,%%cr3"
                     :"=r"(cr3)
                     :
                     :"memory");
}

/**
 * 刷新全局TLB条目。同样会刷新全部条目。
 * 
 * @return 无返回值。
 */
static inline void x86_flush_all_global_tlbs(void)
{
    uintn cr4;
    __asm__ volatile("mov %%cr4,%0":"=r"(cr4));
    __asm__ volatile("mov %0,%%cr4"::"r"(cr4&~((uintn)BIT7)));
    x86_flush_all_tlbs();
    __asm__ volatile("mov %0,%%cr4"::"r"(cr4));
}

#endif /*__AOS_KERNEL_SUPPORT_CACHE_H__*/
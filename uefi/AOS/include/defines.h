/*
 * 模块“aos.uefi”公用声明。
 * 声明了在UEFI阶段使用的公共宏与数据类型。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_DEFINES_H__
#define __AOS_UEFI_DEFINES_H__

#include "config.h"
#include "includes.h"

/*
 * 第一轮检查。
 * 主要根据宏用途初步确定范围。
 */

/*
 * CONFIG_MEMORY_POOL_PAGES检查。
 */
#ifndef CONFIG_MEMORY_POOL_PAGES
#error The macro CONFIG_MEMORY_POOL_PAGES is undefined.
#elif CONFIG_MEMORY_POOL_PAGES<=0
#error The macro CONFIG_MEMORY_POOL_PAGES must be greater than 0.
#elif (CONFIG_MEMORY_POOL_PAGES%64)!=0
#error The macro CONFIG_MEMORY_POOL_PAGES must be a multiple of 64.
#endif /*CONFIG_MEMORY_POOL_PAGES*/

/*
 * CONFIG_MEMORY_PREALLOCATED_PAGES检查。
 */
#ifndef CONFIG_MEMORY_PREALLOCATED_PAGES
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES is undefined.
#elif CONFIG_MEMORY_PREALLOCATED_PAGES<=0
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES must be greater than 0.
#elif CONFIG_MEMORY_PREALLOCATED_PAGES>CONFIG_MEMORY_POOL_PAGES
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES must be less than or equal to CONFIG_MEMORY_POOL_PAGES.
#endif /*CONFIG_MEMORY_PREALLOCATED_PAGES*/

/*
 * 启动标志。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _boot_flags
{
    UINT32 sse    :1; /*SSE支持。*/
    UINT32 sse2   :1; /*SSE2支持。*/
    UINT32 sse3   :1; /*SSE3支持。*/
    UINT32 ssse3  :1; /*SSSE3支持。*/
    UINT32 sse41  :1; /*SSE4.1支持。*/
    UINT32 sse42  :1; /*SSE4.2支持。*/
    UINT32 avx    :1; /*AVX支持。*/
    UINT32 fxsr   :1; /*FXSAVE与FXRSTOR指令支持。*/
    UINT32 xsave  :1; /*XSAVE指令支持。*/
    UINT32 osxsave:1; /*OSXSAVE指令支持。*/
    UINT32 pge    :1; /*PGE支持。*/
} boot_flags;

/*
 * 启动参数。
 * 记录了需要传递到内核的参数。
 */
typedef struct _boot_params
{
    UINTN      bitmap_base; /*位图基址。*/
    UINTN      tlsf_base;   /*TLSF元数据基址。*/
    boot_flags flags;       /*启动标志。*/
} boot_params;

/*
 * 启动核蹦床函数，进入后进入启动核流程，并且不返回。
 *
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
typedef VOID (*aos_boot_trampoline)(boot_params* restrict params);

#endif /*__AOS_UEFI_DEFINES_H__*/
/*
 * 模块“aos.uefi”公用定义。定义了在UEFI阶段使用的公共宏与跨文件数据类型。
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
 * 启动参数。
 */
typedef struct _boot_params
{
    UINTN pool_base; /*内存池基址。*/
    UINTN pool_size; /*内存池长度。*/
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
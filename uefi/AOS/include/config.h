/**
 * 模块配置常量定义。
 * @date 2025-06-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_CONFIG_H__
#define __AOS_UEFI_CONFIG_H__

/**
 * 引导内存池总页数。要求显然大于0。
 */
#define CONFIG_BOOTSTRAP_POOL 64

/**
 * 内核内存池总页数。要求显然大于0。
 */
#define CONFIG_KERNEL_POOL 64

/**
 * 页表内存池总页数。要求为64的倍数，且显然大于0。
 */
#define CONFIG_PAGE_TABLE_POOL 128

/**
 * 强制填充图形界面背景。要求为一个可以表示真假的数值。
 */
#define CONFIG_FORCE_FILL_GRAPHICS_BACKGROUND FALSE

/**
 * 图形界面背景色。要求范围为[0,0xFFFFFF]。
 */
#define CONFIG_GRAPHICS_BACKGROUND_COLOR 0x191970

/**
 * 随机化内核基址功能。
 */
#define CONFIG_RANDOMIZE_BASE TRUE

/**
 * 第一轮检查。
 * 主要根据宏用途初步确定范围。
 */

/**
 * CONFIG_BOOTSTRAP_POOL检查。
 */
#ifndef CONFIG_BOOTSTRAP_POOL
#error The macro CONFIG_BOOTSTRAP_POOL is undefined.
#elif CONFIG_BOOTSTRAP_POOL<=2
#error The macro CONFIG_BOOTSTRAP_POOL must be greater than 2.
#endif /*CONFIG_BOOTSTRAP_POOL*/

/**
 * CONFIG_KERNEL_POOL检查。
 */
#ifndef CONFIG_KERNEL_POOL
#error The macro CONFIG_KERNEL_POOL is undefined.
#elif CONFIG_KERNEL_POOL<=2
#error The macro CONFIG_KERNEL_POOL must be greater than 2.
#endif /*CONFIG_KERNEL_POOL*/

/**
 * CONFIG_PAGE_TABLE_POOL检查。
 */
#ifndef CONFIG_PAGE_TABLE_POOL
#error The macro CONFIG_PAGE_TABLE_POOL is undefined.
#elif CONFIG_PAGE_TABLE_POOL<=0
#error The macro CONFIG_PAGE_TABLE_POOL must be greater than 0.
#elif (CONFIG_PAGE_TABLE_POOL%64)!=0
#error The macro CONFIG_PAGE_TABLE_POOL must be a multiple of 64.
#endif /*CONFIG_PAGE_TABLE_POOL*/

/**
 * CONFIG_GRAPHICS_BACKGROUND_COLOR检查。
 */
#ifndef CONFIG_GRAPHICS_BACKGROUND_COLOR
#error The macro CONFIG_GRAPHICS_BACKGROUND_COLOR is undefined.
#elif CONFIG_GRAPHICS_BACKGROUND_COLOR<0
#error The macro CONFIG_GRAPHICS_BACKGROUND_COLOR must be greater than or equal to 0.
#elif CONFIG_GRAPHICS_BACKGROUND_COLOR>0xFFFFFF
#error The macro CONFIG_GRAPHICS_BACKGROUND_COLOR must be less than or equal to 0xFFFFFF.
#endif /*CONFIG_GRAPHICS_BACKGROUND_COLOR*/

#endif /*__AOS_UEFI_CONFIG_H__*/
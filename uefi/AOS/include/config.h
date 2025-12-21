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
 * 内核栈使用页数。实际处理时会前后各添加一页作为边界，但是这一步骤由内核重构线性区时进行。
 */
#define CONFIG_KERNEL_STACK 16

/**
 * 随机化基址功能。
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
#elif CONFIG_BOOTSTRAP_POOL<=16
#error The macro CONFIG_BOOTSTRAP_POOL must be greater than 16.
#endif /*CONFIG_BOOTSTRAP_POOL*/

/**
 * CONFIG_KERNEL_POOL检查。
 */
#ifndef CONFIG_KERNEL_POOL
#error The macro CONFIG_KERNEL_POOL is undefined.
#elif CONFIG_KERNEL_POOL<=16
#error The macro CONFIG_KERNEL_POOL must be greater than 16.
#endif /*CONFIG_KERNEL_POOL*/

/**
 * CONFIG_PAGE_TABLE_POOL检查。
 */
#ifndef CONFIG_PAGE_TABLE_POOL
#error The macro CONFIG_PAGE_TABLE_POOL is undefined.
#elif CONFIG_PAGE_TABLE_POOL<=64
#error The macro CONFIG_PAGE_TABLE_POOL must be greater than 64.
#elif (CONFIG_PAGE_TABLE_POOL%64)!=0
#error The macro CONFIG_PAGE_TABLE_POOL must be a multiple of 64.
#endif /*CONFIG_PAGE_TABLE_POOL*/

/**
 * CONFIG_KERNEL_STACK检查。
 */
#ifndef CONFIG_KERNEL_STACK
#error The macro CONFIG_KERNEL_STACK is undefined.
#elif CONFIG_KERNEL_STACK<=8
#error The macro CONFIG_KERNEL_STACK must be greater than 8.
#elif (CONFIG_KERNEL_STACK%8)!=0
#error The macro CONFIG_KERNEL_STACK must be a multiple of 8.
#endif /*CONFIG_KERNEL_STACK*/

/**
 * CONFIG_RANDOMIZE_BASE检查。
 */
#ifndef CONFIG_RANDOMIZE_BASE
#error The macro CONFIG_RANDOMIZE_BASE is undefined.
#endif /*CONFIG_RANDOMIZE_BASE*/

#endif /*__AOS_UEFI_CONFIG_H__*/
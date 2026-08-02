/**
 * 模块配置常量定义。
 * @date 2026-06-28
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_CONFIG_H__
#define __AOS_UEFI_CONFIG_H__

/**
 * 引导内存池总页数。
 */
#define CONFIG_BOOTSTRAP_POOL 64

/**
 * 内核内存池总页数。
 */
#define CONFIG_KERNEL_POOL 64

/**
 * 页表内存池总页数。
 */
#define CONFIG_PAGE_TABLE_POOL 128

/**
 * 内核栈使用页数。
 */
#define CONFIG_KERNEL_STACK 16

/**
 * 随机化基址功能。
 */
#define CONFIG_RANDOMIZE_BASE TRUE

/**
 * 最低自由内存数目。
 */
#define CONFIG_MINIMUM_FREE_PAGES (SIZE_512MB/EFI_PAGE_SIZE)

#endif /*__AOS_UEFI_CONFIG_H__*/
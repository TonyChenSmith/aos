/*
 * 模块“aos.uefi”配置。定义了该模块主要功能的宏配置变量。
 * @date 2025-06-05
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_CONFIG_H__
#define __AOS_UEFI_CONFIG_H__

/*
 * 模块“aos.uefi”的内存池总页数。要求为64的倍数，且显然大于0。
 */
#define CONFIG_MEMORY_POOL_PAGES 256

/*
 * 模块“aos.uefi”的内存池预分配页数。要求小于总页数，且显然大于0。
 */
#define CONFIG_MEMORY_PREALLOCATED_PAGES 16

#endif /*__AOS_UEFI_CONFIG_H__*/
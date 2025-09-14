/* 
 * 模块“aos.uefi”配置。
 * 定义了主要功能的宏配置变量。
 * @date 2025-06-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_CONFIG_H__
#define __AOS_UEFI_CONFIG_H__

/* 
 * 内存池总页数。要求为64的倍数，且显然大于0。
 */
#define CONFIG_MEMORY_POOL_PAGES 256

/* 
 * 内存池预分配页数。要求小于总页数，且显然大于0。
 */
#define CONFIG_MEMORY_PREALLOCATED_PAGES 8

/* 
 * 强制填充图形界面背景。要求为一个可以表示真假的数值。
 */
#define CONFIG_FORCE_FILL_GRAPHICS_BACKGROUND FALSE

/* 
 * 图形界面背景色。要求范围为[0,0xFFFFFF]。
 */
#define CONFIG_GRAPHICS_BACKGROUND_COLOR 0x191970

#endif /*__AOS_UEFI_CONFIG_H__*/
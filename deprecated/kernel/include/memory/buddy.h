/*
 * 伙伴系统结构。
 * @date 2024-12-12
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_MEMORY_BUDDY_H__
#define __AOS_MEMORY_BUDDY_H__

#include "util/bitmap_pool.h"

/*伙伴列表*/
typedef struct _buddy_list
{
	bitmap_pool* pool; /*分配池*/
	handle blocks[52];  /*空闲内存块组*/
} buddy_list;

#endif /*__AOS_MEMORY_BUDDY_H__*/
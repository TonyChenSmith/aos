/*
 * 位映射池的功能定义。
 * @date 2024-12-13
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_MEMORY_BOOT_BITMAP_H__
#define __AOS_BOOT_MEMORY_BOOT_BITMAP_H__

#include "util/bitmap_pool.h"
#include "module/init.h"

/*
 * 位映射池初始化。需要提前分配好的内存。
 *
 * @param global 全局函数表。
 * @param pool	 位映射池内存。
 * @param psize  位映射池大小，即结点数量。
 * @param nsize	 结点大小。	
 *
 * @return 初始化后返回真，参数出错返回假。
 */
extern bool boot_bitmap_pool_init(const boot_function_table* restrict global,void* restrict pool,const uint16 psize,const uint32 nsize);

/*
 * 位映射池结点分配。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 *
 * @return 分配好的结点，分配失败时返回未定义。
 */
extern handle boot_bitmap_pool_alloc(bitmap_pool* restrict pool);

/*
 * 位映射池结点释放。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 * @param node 需要释放的结点。
 *
 * @return 无返回值。
 */
extern void boot_bitmap_pool_free(bitmap_pool* restrict pool,const handle node);

/*
 * 位映射池获得结点内容。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 * @param node 需要读取的结点。
 *
 * @return 结点地址，读取失败时返回未定义。
 */
extern void* boot_bitmap_pool_content(bitmap_pool* restrict pool,const handle node);

/*
 * 位映射池获得可用结点数目。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 *
 * @return 可用结点数目。
 */
extern uintn boot_bitmap_pool_available(bitmap_pool* restrict pool);

/*
 * 位映射池检查需求结点数目。
 *
 * @param pool	  位映射池。应为位映射池链表起始。
 * @param require 需求结点数目
 *
 * @return 有足够结点返回真，没有返回假。
 */
extern bool boot_bitmap_pool_require(bitmap_pool* restrict pool,const uintn require);

#endif /*__AOS_BOOT_MEMORY_BOOT_BITMAP_H__*/
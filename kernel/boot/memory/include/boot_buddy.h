/*
 * 伙伴系统结构。
 * @date 2024-12-12
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_MEMORY_BOOT_BUDDY_H__
#define __AOS_BOOT_MEMORY_BOOT_BUDDY_H__

#include "boot_bitmap.h"

/*
 * 结点清空。
 *
 * @param node 结点地址。
 * 
 * @return 无返回值。
 */
typedef uintn (*boot_node_clear)(void* node);

/*
 * 结点比较。
 *
 * @param left	左结点。
 * @param right	右结点。
 * 
 * @return -1小于，0等于，1大于。其他状态假设不可知。
 */
typedef int32 (*boot_node_compare)(void* left,void* right);

/*
 * 链表添加尾结点。
 *
 * @param pool	位映射池。
 * @param list	链表。
 * @param node	结点。
 * @param clear	结点清空函数。
 *
 * @return 成功返回真，参数异常返回假。
 */
extern bool boot_list_bitmap_add(bitmap_pool* restrict pool,linked_list* restrict list,handle node,const boot_node_clear clear);

/*
 * 链表添加尾结点。
 *
 * @param pool	位映射池。
 * @param list	链表。
 * @param node	结点。
 * @param clear	结点清空函数。
 *
 * @return 成功返回真，参数异常返回假。
 */
extern bool boot_list_bitmap_add(bitmap_pool* restrict pool,linked_list* restrict list,handle node,const boot_node_clear clear);

/*
 * 链表删除结点。
 *
 * @param pool 位映射池。
 * @param list 链表。
 * @param node 结点。
 *
 * @return 无返回值。
 */
extern void boot_list_bitmap_remove(bitmap_pool* restrict pool,linked_list* restrict list,handle node);

/*
 * 伙伴系统最小下限内存申请。
 *
 * @param buddy	伙伴列表。
 * @param min	最小地址范围，闭区间。
 * @param pages	申请页数。
 *
 * @return 成功返回基址，失败返回未定义。
 */
extern uintn boot_buddy_alloc_min(buddy_list* restrict buddy,const uintn min,const uintn pages);

/*
 * 伙伴系统最大上限内存申请。
 *
 * @param buddy	伙伴列表。
 * @param max	最大地址范围，开区间。
 * @param pages	申请页数。
 *
 * @return 成功返回基址，失败返回未定义。
 */
extern uintn boot_buddy_alloc_max(buddy_list* restrict buddy,const uintn max,const uintn pages);

/*
 * 伙伴系统区间内存申请。
 *
 * @param buddy	伙伴列表。
 * @param min	最小地址范围，闭区间。
 * @param max	最大地址范围，开区间。
 * @param pages	申请页数。
 *
 * @return 成功返回基址，失败返回未定义。
 */
extern uintn boot_buddy_alloc_range(buddy_list* restrict buddy,const uintn min,const uintn max,const uintn pages);

/*
 * 伙伴系统内存释放。
 *
 * @param buddy	伙伴列表。
 * @param base	释放内存基址。
 * @param pages	释放内存页数。
 *
 * @return 成功返回真，资源不足返回假。
 */
extern bool boot_buddy_free(buddy_list* restrict buddy,uintn base,uintn pages);

#endif /*__AOS_BOOT_MEMORY_BOOT_BUDDY_H__*/
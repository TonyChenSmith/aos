/*
 * 红黑树结构功能定义。
 * @date 2025-01-03
 */
#ifndef __AOS_BOOT_MEMORY_BOOT_TREE_H__
#define __AOS_BOOT_MEMORY_BOOT_TREE_H__

#include "util/tree.h"
#include "boot_bitmap.h"

/*
 * 红黑树比较函数。
 *
 * @param a 比较左式。
 * @param b 比较右式。
 *
 * @return a小于b返回负数，a等于b返回零，a大于b返回正数。
 */
typedef int (*boot_tree_compare)(handle a,handle b);

/*
 * 红黑树结点插入。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param node 结点句柄。
 *
 * @return 成功返回真，失败返回假。
 */
extern bool boot_tree_insert(bitmap_pool* restrict pool,handle* restrict root,const boot_tree_compare compare,const handle node);

/*
 * 红黑树结点删除。使用者有责任保证结点在树内。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param node 结点句柄。
 *
 * @return 无返回值。
 */
extern void boot_tree_remove(bitmap_pool* restrict pool,handle* restrict root,handle node);

/*
 * 红黑树结点查找。
 *
 * @param pool	  位映射池。
 * @param root	  树根结点。
 * @param compare 比较函数。
 * @param key	  查找键。
 *
 * @return 返回对应句柄，或未定义。
 */
extern handle boot_tree_find(bitmap_pool* restrict pool,handle* restrict root,const boot_tree_compare compare,handle key);

#endif /*__AOS_BOOT_MEMORY_BOOT_TREE_H__*/
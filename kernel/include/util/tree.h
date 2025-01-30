/*
 * 红黑树的结构。
 * @date 2025-01-02
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UTIL_TREE_H__
#define __AOS_UTIL_TREE_H__

#include "basic_type.h"

/*红黑树结点*/
typedef struct _tree_node
{
	handle parent;	/*父节点句柄和颜色*/
	handle left;	/*左子结点句柄*/
	handle right;	/*右子节点句柄*/
	uint8 color;	/*结点颜色*/
	uint8 unused1;	/*未用1*/
	uint16 unused2; /*未用2*/
	uint32 unused3; /*未用3*/
} tree_node;

/*黑色*/
#define TREE_BLACK 0

/*红色*/
#define TREE_RED UINT8_MAX

#endif /*__AOS_UTIL_TREE_H__*/
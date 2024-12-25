/*
 * 双向链表的结构。
 * @date 2024-12-14
 */
#ifndef __AOS_UTIL_LINKED_LIST_H__
#define __AOS_UTIL_LINKED_LIST_H__

#include "basic_type.h"

/*链表*/
typedef struct _linked_list
{
	handle head; /*头结点*/
	handle tail; /*尾结点*/
} linked_list;

/*链表结点*/
typedef struct _list_node
{
	handle prev; /*前一结点*/
	handle next; /*后一结点*/
} list_node;

#endif /*__AOS_UTIL_LINKED_LIST_H__*/
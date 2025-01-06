/*
 * 伙伴系统结构。
 * @date 2024-12-12
 */
#ifndef __AOS_MEMORY_BUDDY_H__
#define __AOS_MEMORY_BUDDY_H__

#include "util/bitmap_pool.h"

/*伙伴列表*/
typedef struct _buddy_list
{
	bitmap_pool* pool; /*分配池*/
	handle block[52];  /*内存块树*/
} buddy_list;

#endif /*__AOS_MEMORY_BUDDY_H__*/
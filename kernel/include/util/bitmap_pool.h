/*
 * 位映射池的结构。
 * @date 2024-12-13
 */
#ifndef __AOS_UTIL_BITMAP_POOL_H__
#define __AOS_UTIL_BITMAP_POOL_H__

#include "util/linked_list.h"

/*位映射池*/
typedef struct _bitmap_pool
{
	list_node node; /*链表结点*/
	uint16 psize;	/*单个池结点容量*/
	uint16 free;	/*可分配结点数*/
	uint32 nsize;	/*单个结点大小*/
	uintn offset;	/*分配内存偏移*/
} bitmap_pool;

/*位映射作用大小*/
#define bitmap_pool_bitmap_size(psize) (sizeof(bitmap_pool)+(psize>>3)+(psize&0x7?1:0))

/*位映射作用页面*/
#define bitmap_pool_bitmap_page(psize) ((bitmap_pool_bitmap_size(psize)>>12)+(bitmap_pool_bitmap_size(psize)&0xFFF?1:0))

/*结点占用大小*/
#define bitmap_pool_node_size(psize,nsize) (psize*nsize)

/*结点占用页面*/
#define bitmap_pool_node_page(psize,nsize) ((bitmap_pool_node_size(psize,nsize)>>12)+(bitmap_pool_node_size(psize,nsize)&0xFFF?1:0))

/*位映射池占用大小*/
#define bitmap_pool_page(psize,nsize) (bitmap_pool_bitmap_page(psize)+bitmap_pool_node_page(psize,nsize))

#endif /*__AOS_UTIL_BITMAP_POOL_H__*/
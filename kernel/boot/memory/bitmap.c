/*
 * 位映射池内存模块实现。这是一种内存模块特殊使用的内存分配方式，基于相对定位方式，所以不公开到基础模块。
 * @date 2024-12-13
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/boot_bitmap.h"

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
extern bool boot_bitmap_pool_init(const boot_function_table* restrict global,void* restrict pool,const uint16 psize,const uint32 nsize)
{
	if(psize==0||nsize==0||(uintn)pool==UINT64_MAX)
	{
		return false;
	}

	bitmap_pool* bpinfo=(bitmap_pool*)pool;
	bpinfo->node.prev=HANDLE_UNDEFINED;
	bpinfo->node.prev=HANDLE_UNDEFINED;
	bpinfo->psize=psize;
	bpinfo->free=psize;
	bpinfo->nsize=nsize;
	bpinfo->offset=bitmap_pool_bitmap_page(psize)<<12;

	uint8* bitmap=(uint8*)((uintn)pool+sizeof(bitmap_pool));
	uintn limit=bpinfo->offset-sizeof(bitmap_pool);
	uintn spilt=bitmap_pool_bitmap_size(psize)-sizeof(bitmap_pool);
	global->util->memset(bitmap,0,spilt);
	global->util->memset(&bitmap[spilt],UINT32_MAX,limit-spilt);
	if(psize&0x7)
	{
		bitmap[spilt-1]=UINT8_MAX;
		limit=psize&0x7;
		while(limit>0)
		{
			bitmap[spilt-1]=bitmap[spilt-1]<<1;
			limit--;
		}
	}
	return true;
}

/*
 * 获取索引对应的当前位映射池。会通过对池链表的遍历获得对应索引的位映射池。
 *
 * @param pool	位映射池。应为位映射池链表起始。
 * @param index	位映射池索引。
 *
 * @return 对应当前位映射池，或未定义。
 */
static bitmap_pool* boot_bitmap_pool_current(bitmap_pool* restrict pool,uintn index)
{
	while(index>0)
	{
		if(pool->node.next==HANDLE_UNDEFINED)
		{
			return (bitmap_pool*)HANDLE_UNDEFINED;
		}
		pool=(bitmap_pool*)pool->node.next;
		index--;
	}
	return pool;
}

/*
 * 位映射池结点分配。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 *
 * @return 分配好的结点，分配失败时返回未定义。
 */
extern handle boot_bitmap_pool_alloc(bitmap_pool* restrict pool)
{
	uintn pindex=0;

	while(pool->free==0)
	{
		if(pool->node.next==HANDLE_UNDEFINED)
		{
			return HANDLE_UNDEFINED;
		}
		pool=(bitmap_pool*)pool->node.next;
		pindex++;
	}

	uint16 index=0;
	uint16 bindex=0;
	uint8* bitmap=(uint8*)((uintn)pool+sizeof(bitmap_pool));

	while(index<pool->psize)
	{
		if(bitmap[bindex]&(1<<index))
		{
			index++;
			if(index>=8)
			{
				bindex++;
				index=0;
			}
		}
		else
		{
			bitmap[bindex]=bitmap[bindex]|(1<<index);
			pool->free--;
			/*高48位为池索引，代表在链表上的位置，低16位为池内索引*/
			return (pindex<<16)+((uintn)bindex<<3)+index;
		}
	}

	/*按检查理论上不可达*/
	return HANDLE_UNDEFINED;
}

/*
 * 位映射池结点释放。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 * @param node 需要释放的结点。
 *
 * @return 无返回值。
 */
extern void boot_bitmap_pool_free(bitmap_pool* restrict pool,const handle node)
{
	uint16 nindex=node&0xFFFF;
	pool=boot_bitmap_pool_current(pool,node>>16);
	if((uintn)pool==HANDLE_UNDEFINED||nindex>=pool->psize)
	{
		return;
	}

	uint8* bitmap=(uint8*)((uintn)pool+sizeof(bitmap_pool));
	uint16 bindex=(nindex>>3);

	if(bitmap[bindex]&(1<<(nindex&0x7)))
	{
		bitmap[bindex]=bitmap[bindex]&(~(1<<(nindex&0x7)));
		pool->free++;
	}
}

/*
 * 位映射池获得结点内容。其返回索引所指定的地址。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 * @param node 需要读取的结点。
 *
 * @return 结点地址，读取失败时返回未定义。
 */
extern void* boot_bitmap_pool_content(bitmap_pool* restrict pool,const handle node)
{
	if(node==HANDLE_UNDEFINED)
	{
		return (void*)HANDLE_UNDEFINED;
	}

	uintn nindex=node&0xFFFF;
	pool=boot_bitmap_pool_current(pool,node>>16);
	if((uintn)pool==HANDLE_UNDEFINED||nindex>=pool->psize)
	{
		return (void*)HANDLE_UNDEFINED;
	}

	/*计算方法其实和数组调用一致*/
	return (void*)((uintn)pool+pool->offset+nindex*pool->nsize);
}

/*
 * 位映射池获得可用结点数目。其会遍历整个位映射池列表，速率较慢。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 *
 * @return 可用结点数目。
 */
extern uintn boot_bitmap_pool_available(bitmap_pool* restrict pool)
{
	uintn available=0;
	while((uintn)pool!=HANDLE_UNDEFINED)
	{
		available=available+pool->free;
		pool=(bitmap_pool*)pool->node.next;
	}
	return available;
}

/*
 * 位映射池检查需求结点数目。其会遍历足够的位映射池，数目足够时即刻返回。
 *
 * @param pool	  位映射池。应为位映射池链表起始。
 * @param require 需求结点数目
 *
 * @return 有足够结点返回真，没有返回假。
 */
extern bool boot_bitmap_pool_require(bitmap_pool* restrict pool,const uintn require)
{
	uintn available=0;
	while((uintn)pool!=HANDLE_UNDEFINED)
	{
		available=available+pool->free;
		if(available>=require)
		{
			return true;
		}
		pool=(bitmap_pool*)pool->node.next;
	}
	return false;
}
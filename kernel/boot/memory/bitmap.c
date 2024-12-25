/*
 * 位映射池启动内核实现。
 * @date 2024-12-13
 */
#include "include/boot_bitmap.h"

/*
 * 位映射池初始化。需要提前分配好的内存。
 *
 * @param pool	位映射池内存。
 * @param psize 位映射池大小，即结点数量。
 * @param nsize	结点大小。
 * @param bbft	基础模块函数表。	
 *
 * @return 初始化后返回真，参数出错返回假。
 */
extern bool boot_bitmap_pool_init(void* restrict pool,const uint16 psize,const uintn nsize,const boot_base_functions* bbft)
{
	if(psize==0||nsize==0||(uintn)pool==UINT64_MAX)
	{
		return false;
	}
	bitmap_pool* bpinfo=(bitmap_pool*)pool;
	bpinfo->node.prev=HANDLE_UNDEFINED;
	bpinfo->node.prev=HANDLE_UNDEFINED;
	bpinfo->size=psize;
	bpinfo->free=psize;
	bpinfo->offset=bitmap_pool_bitmap_page(psize)<<12;
	uint8* bitmap=(uint8*)((uintn)pool+sizeof(bitmap_pool));
	uintn limit=bpinfo->offset-sizeof(bitmap_pool);
	uintn spilt=bitmap_pool_bitmap_size(psize)-sizeof(bitmap_pool);
	bbft->boot_memset(bitmap,0,spilt);
	bbft->boot_memset(&bitmap[spilt],UINT32_MAX,limit-spilt);
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
 * 获取索引对应的当前位映射池。
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
	while(index<pool->size)
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
			return (pindex<<16)+((uintn)bindex<<3)+index;
		}
	}
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
	if((uintn)pool==HANDLE_UNDEFINED||nindex>=pool->size)
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
 * 位映射池获得结点内容。
 *
 * @param pool 位映射池。应为位映射池链表起始。
 * @param node 需要读取的结点。
 * @param size 结点大小。
 *
 * @return 结点地址，读取失败时返回未定义。
 */
extern void* boot_bitmap_pool_content(bitmap_pool* restrict pool,const handle node,const uintn size)
{
	uintn nindex=node&0xFFFF;
	pool=boot_bitmap_pool_current(pool,node>>16);
	if((uintn)pool==HANDLE_UNDEFINED||nindex>=pool->size)
	{
		return (void*)HANDLE_UNDEFINED;
	}
	return (void*)((uintn)pool+pool->offset+nindex*size);
}
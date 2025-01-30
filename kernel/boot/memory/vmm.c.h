/*
 * 虚拟内存伙伴系统。
 * @date 2024-12-25
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/vmm.h"
#include "basic_type.h"
#include "include/boot_bitmap.h"
#include "memory/page_frame.h"

/*伙伴系统*/
static buddy_list buddy={
	.pool=(void*)HANDLE_UNDEFINED,
	.block={[0 ... 51]={HANDLE_UNDEFINED,HANDLE_UNDEFINED}}
};

/*记录链表，0为已分配链表，1为拆分属性结点，2为映射链表，只分配不删除*/
static linked_list record_list[3]={[0 ... 2]={HANDLE_UNDEFINED,HANDLE_UNDEFINED}};

/*
 * 虚拟内存位映射池初始化。
 *
 * @param param 启动参数结构。
 * @param bbft	基础模块函数表。
 *
 * @return 无返回值。
 */
static void boot_vmm_bitmap_pool_init(const boot_params* restrict param,const boot_base_functions* bbft)
{
	buddy.pool=(void*)boot_pmm_alloc(MALLOC_ANY,0,0,BOOT_VMP_PAGE,AOS_KERNEL_DATA);
	if((uintn)buddy.pool==HANDLE_UNDEFINED)
	{
		/*可用内存不足，可能是参数设置问题，也有可能是预设内存池大小问题*/
		DEBUG_START
		/*错误：物理内存空间不足以分配该数目的虚拟页框结点，请尝试重新配置BOOT_VIRTUAL_MEMORY_POOL。系统因此将在此处关机。*/
		#define BOOT_VM_ERROR_MSG "Error:There is not enough physical memory space to allocate that number of virtual page frame nodes.Try reconfiguring BOOT_VIRTUAL_MEMORY_POOL.The system will therefore shut down here.\n"
		bbft->boot_writeport(BOOT_VM_ERROR_MSG,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(BOOT_VM_ERROR_MSG)-1);
		DEBUG_END
		bbft->boot_ms_call_4(param->runtime->reset_system,EFI_RESET_SHUTDOWN,EFI_OUT_OF_RESOURCES,0,0);
		__builtin_unreachable();
	}
	boot_bitmap_pool_init(buddy.pool,BOOT_VIRTUAL_MEMORY_POOL,sizeof(virtual_page_frame),bbft);
}

/*
 * 虚拟页框结点清空。
 *
 * @param node 结点。
 * 
 * @return 无返回值。
 */
static void boot_vmm_clear(const handle node)
{
	__builtin_memset_inline(boot_bitmap_pool_content(buddy.pool,node,sizeof(virtual_page_frame)),0,sizeof(virtual_page_frame));
}

/*
 * 页框区间比较。
 *
 * @param lbase	 左页框基址。
 * @param lpages 左页框页数。
 * @param rbase	 右页框基址。
 * @param rpages 右页框页数。
 * 
 * @return -1小于，0等于或左覆盖右，1大于，2左右交错，3右左交错，4右覆盖左。
 */
static int32 boot_vmm_compare(const uintn lbase,const uintn lpages,const uintn rbase,const uintn rpages)
{
	uintn la=lbase;
	uintn lb=la-1+(lpages<<12);
	uintn ra=rbase;
	uintn rb=ra-1+(rpages<<12);
	if(lb<ra)
	{
		return -1;
	}
	else if(rb<la)
	{
		return 1;
	}
	else if(la<=ra&&rb<=lb)
	{
		return 0;
	}
	else if(ra<=la&&lb<=rb&&!(ra==la&&rb==lb))
	{
		return 4;
	}
	else if(la<ra&&ra<lb&&lb<rb)
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

/*
 * 链表添加结点。
 *
 * @param list 链表。
 * @param node 结点。需要提前初始化。
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_vmm_list_add(linked_list* restrict list,const handle node)
{
	if(list->head==HANDLE_UNDEFINED)
	{
		list->head=node;
		list->tail=node;
		return true;
	}
	else
	{
		uintn head=list->head;
		virtual_page_frame* left;
		virtual_page_frame* right;
		right=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(virtual_page_frame));
		while(head!=HANDLE_UNDEFINED)
		{
			left=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(virtual_page_frame));
			switch(boot_vmm_compare(left->vbase,left->pages,right->vbase,right->pages))
			{
				case -1:
					/*小于，到下一结点*/
					head=left->next;
					continue;
				case 1:
					/*大于，添加结点*/
					if(left->prev!=HANDLE_UNDEFINED)
					{
						virtual_page_frame* pc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,left->prev,sizeof(virtual_page_frame));
						pc->next=node;
						right->prev=left->prev;
						right->next=head;
						left->prev=node;
					}
					else
					{
						list->head=node;
						left->prev=node;
						right->next=head;
					}
					return true;
				default:
					return false;
			}
		}
		/*全小于，添加到末尾*/
		left=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,list->tail,sizeof(virtual_page_frame));
		left->next=node;
		right->prev=list->tail;
		list->tail=node;
		return true;
	}
}

/*
 * 链表删除结点。
 *
 * @param list 链表。
 * @param node 结点。
 *
 * @return 有删除行为返回真，无行为返回假。
 */
static bool boot_vmm_list_remove(linked_list* restrict list,const handle node)
{
	handle head=list->head;
	while(head!=HANDLE_UNDEFINED)
	{
		virtual_page_frame* hc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(virtual_page_frame));
		if(head==node)
		{
			if(hc->prev==HANDLE_UNDEFINED)
			{
				/*头结点或唯一结点*/
				if(hc->next==HANDLE_UNDEFINED)
				{
					/*唯一结点*/
					list->head=HANDLE_UNDEFINED;
					list->tail=HANDLE_UNDEFINED;
				}
				else
				{
					/*头结点*/
					list->head=hc->next;
					hc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->next,sizeof(virtual_page_frame));
					hc->prev=HANDLE_UNDEFINED;
				}
			}
			else if(hc->next==HANDLE_UNDEFINED) 
			{
				/*尾结点*/
				list->tail=hc->prev;
				hc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->prev,sizeof(virtual_page_frame));
				hc->next=HANDLE_UNDEFINED;
			}
			else
			{
				/*中间结点*/
				virtual_page_frame* nc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->next,sizeof(physical_page_frame));
				virtual_page_frame* pc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->prev,sizeof(physical_page_frame));
				pc->next=hc->next;
				nc->prev=hc->prev;
			}
			boot_bitmap_pool_free(buddy.pool,node);
			return true;
		}
		else
		{
			head=hc->next;
		}
	}
	return false;
}

/*
 * 申请一个结点。
 *
 * @param pbase	物理内存基址。
 * @param vbase	虚拟内存基址。
 * @param pages	内存页数。
 * @param type	内存类型。
 * @param attr	内存属性。
 * @param pid	所属处理器。
 *
 * @return 结点句柄，申请失败返回未定义。
 */
static handle boot_vmm_node_alloc(const uintn pbase,const uintn vbase,const uintn pages,const uint16 type,const uint16 attr,const uint32 pid)
{
	handle result=boot_bitmap_pool_alloc(buddy.pool);
	if(result==HANDLE_UNDEFINED)
	{
		return result;
	}
	virtual_page_frame* content=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,result,sizeof(virtual_page_frame));
	content->prev=HANDLE_UNDEFINED;
	content->next=HANDLE_UNDEFINED;
	content->pbase=pbase;
	content->vbase=vbase;
	content->pages=pages;
	content->attr=attr;
	content->pid=pid;
	content->type=type;
	return result;
}

/*
 * 检查可用结点数目是否符合期待。
 *
 * @param expect 期待结点数目。
 *
 * @return 如果结点足够返回真，如果结点不足返回假。
 */
static bool boot_vmm_expect(const uintn expect)
{
	bool result=boot_bitmap_pool_expect(buddy.pool,expect);
	if(result)
	{
		return true;
	}
	else
	{
		/*尝试增加池子*/
		return false;
	}
}

/*
 * 对结点进行归并。将低层级块归并到高层级，仅适用于伙伴链表的内存块。
 *
 * @param block	内存块索引，不大于51。
 * @param pid	所属处理器编号。
 *
 * @return 无返回值。
 */
static void boot_vmm_buddy_merge(const uintn block,const uint32 pid)
{
	if(block>=51||buddy.block[block].head==HANDLE_UNDEFINED)
	{
		return;
	}
	uintn mask=((uintn)1<<(block+13))-1;
	uintn head=buddy.block[block].head;
	virtual_page_frame* hc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(virtual_page_frame));
	while(hc->next!=HANDLE_UNDEFINED)
	{
		virtual_page_frame* nc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->next,sizeof(virtual_page_frame));
		if(!(hc->vbase&mask))
		{
			if(nc->vbase==hc->vbase+(hc->pages<<12))
			{
				uintn tbase=hc->vbase;
				uintn tpage=hc->pages<<1;
				handle ra=head;
				handle rb=hc->next;
				boot_vmm_list_remove(&buddy.block[block],ra);
				boot_vmm_list_remove(&buddy.block[block],rb);
				head=boot_vmm_node_alloc(HANDLE_UNDEFINED,tbase,tpage,AOS_AVAILABLE,0,pid);
				boot_vmm_list_add(&buddy.block[block+1],head);
				head=buddy.block[block].head;
				if(head==HANDLE_UNDEFINED)
				{
					return;
				}
				else
				{
					hc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(virtual_page_frame));
					continue;
				}
			}
		}
		head=hc->next;
		hc=nc;
	}
}

/*
 * 在伙伴系统中释放空闲内存。
 *
 * @param base	内存基址。
 * @param pages	内存页数。
 * @param pid	所属处理器编号。
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_vmm_buddy_free(uintn base,uintn pages,const uint32 pid)
{
	while(pages>0)
	{
		uint8 mask=__builtin_ctzg(base,(int)sizeof(uintn)*8-1);
		uint8 index=mask-12;
		uintn page=(uintn)1<<index;
		while(pages<page)
		{
			index--;
			page=(uintn)1<<index;
		}
		if(!boot_vmm_expect(1))
		{
			return false;
		}
		handle node=boot_vmm_node_alloc(HANDLE_UNDEFINED,base,page,AOS_AVAILABLE,0,pid);
		if(!boot_vmm_list_add(&buddy.block[index],node))
		{
			return false;
		}
		else
		{
			base=base+(page<<12);
			pages=pages-page;
		}
	}
	return true;
}

/*
 * 在伙伴系统中分割空闲内存。
 *
 * @param block	内存块索引，大于0。
 * @param node	结点。
 * @param pid	所属处理器编号。
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_vmm_buddy_spilt(const uintn block,const handle node,const uint32 pid)
{
	if(block==0||block>51||node==HANDLE_UNDEFINED)
	{
		return false;
	}
	virtual_page_frame* nc=(virtual_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(virtual_page_frame));
	uintn tbase=nc->vbase;
	uintn tpage=nc->pages>>1;
	if(!boot_vmm_expect(1))
	{
		return false;
	}
	boot_vmm_list_remove(&buddy.block[block],node);
	handle a=boot_vmm_node_alloc(HANDLE_UNDEFINED,tbase,tpage,AOS_AVAILABLE,0,pid);
	handle b=boot_vmm_node_alloc(HANDLE_UNDEFINED,tbase+(tpage<<12),tpage,AOS_AVAILABLE,0,pid);
	boot_vmm_list_add(&buddy.block[block-1],a);
	boot_vmm_list_add(&buddy.block[block-1],b);
	return true;
}

/*
 * 内存地址相对结点定位。结果为结点比较地址。
 *
 * @param base	结点基址。
 * @param pages	结点页数。
 * @param addr	内存地址。
 * 
 * @return -1小于，0包含，1大于。
 */
static int32 boot_vmm_position(const uintn base,const uintn pages,const uintn addr)
{
	if(addr<base)
	{
		return 1;
	}
	else if(addr<=(base-1+(pages<<12))) 
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
 * 将空闲内存记录到已分配内存中。
 *
 * @param content 结点内容。
 * @param node	  结点索引。
 * @param block	  内存块链表。
 * @param pbase	  物理内存基址。
 * @param pages	  内存页数。
 * @param type	  内存类型。
 * 
 *
 * @return 已分配地址，或未定义。
 */
static uintn boot_vmm_record(const virtual_page_frame* restrict content,const uintn node,const uintn block,const uintn pbase,const uintn pages,const memory_type type,const uint16 attr,const uint32 pid)
{
	if(!boot_vmm_expect(1))
	{
		return HANDLE_UNDEFINED;
	}
	handle rnode=boot_vmm_node_alloc(pbase,content->vbase,pages,type,attr,pid);
	boot_vmm_list_add(&record_list[0],rnode);
	uintn rbase=content->vbase;
	uintn fbase=content->vbase+(pages<<12);
	uintn fpage=content->pages-pages;
	boot_vmm_list_remove(&buddy.block[block],node);
	if(fpage>0)
	{
		boot_vmm_buddy_free(fbase,fpage,pid);
	}
	return rbase;
}
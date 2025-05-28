/*
 * 物理内存管理系统(PMMS)。
 * @date 2025-01-03
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/pmm.h"
#include "basic_type.h"
#include "include/tree.h"
#include "memory/memory_defs.h"
#include "memory/page_frame.h"
#include "params.h"

/*物理内存管理伙伴系统*/
static buddy_list buddy={
	.pool=(void*)HANDLE_UNDEFINED,
	.blocks={[0 ... 51]=HANDLE_UNDEFINED}
};

/*已使用内存块树数组*/
static handle used[3]={[0 ... 2]=HANDLE_UNDEFINED};

/*
 * 物理内存位映射池初始化。
 *
 * @param global 全局函数表。
 * @param params 启动核参数。
 *
 * @return 无返回值。
 */
static void boot_pmm_bitmap_pool_init(const boot_function_table* restrict global,const boot_params* restrict params)
{
	if(params->blocks[BOOT_PMMS_BLOCK].pages<BOOT_PMP_PAGE)
	{
		/*可用内存不足，可能且仅可能是参数设置问题*/
		DEBUG_START
		/*错误：启动核初始化失败。在UEFI核预置的1MB物理内存管理系统内存块不能支持BOOT_PHYSICAL_MEMORY_POOL所指定数目的物理页框结点分配。请尝试修改BOOT_PHYSICAL_MEMORY_POOL。系统将执行保护性停机。*/
		#define BOOT_PMMS_ERROR_MSG "Error: Boot kernel initialization failed. The UEFI pre-allocated 1MB physical memory block for the physical memory management system could not support the allocation and management of the specified number of physical page frames as defined by BOOT_PHYSICAL_MEMORY_POOL. Please try modifying BOOT_PHYSICAL_MEMORY_POOL. The system will perform a protective shutdown.\n"
		global->util->writeport(BOOT_PMMS_ERROR_MSG,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(BOOT_PMMS_ERROR_MSG)-1);
		DEBUG_END

		global->util->ms_call_4(params->runtime->reset_system,EFI_RESET_SHUTDOWN,EFI_OUT_OF_RESOURCES,0,0);
		__builtin_unreachable();
	}
	boot_bitmap_pool_init(global,(void*)params->blocks[BOOT_PMMS_BLOCK].base,BOOT_PHYSICAL_MEMORY_POOL,sizeof(physical_page_frame));
	buddy.pool=(bitmap_pool*)params->blocks[BOOT_PMMS_BLOCK].base;
}

/*
 * 物理页框区间比较函数。用于红黑树插入。
 *
 * @param left	左页框。
 * @param right	右页框。
 * 
 * @return -1小于，0等于或左覆盖右，1大于，2左右交错，3右左交错，4右覆盖左。在PMMS中原则上不出现后三种情况。
 */
static int boot_pmm_compare(const handle left,const handle right)
{
	const physical_page_frame* leftc=(physical_page_frame*)left;
	const physical_page_frame* rightc=(physical_page_frame*)right;

	uintn la=leftc->base;
	uintn lb=la-1+(leftc->pages<<12);
	uintn ra=rightc->base;
	uintn rb=ra-1+(rightc->pages<<12);

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
 * 将EFI内存类型转换为AOS内存类型。
 *
 * @param efimt EFI内存类型。
 *
 * @return 对应AOS内存类型。
 */
static memory_type boot_pmm_efi2aos(const uint32 efimt)
{
	switch(efimt)
	{
		case EFI_RESERVED_MEMORY_TYPE:
		case EFI_UNACCEPTED_MEMORY_TYPE:
		case EFI_PERSISTENT_MEMORY:
		case EFI_PAL_CODE:
		case EFI_UNUSABLE_MEMORY:
		default:
			/*一般非正常内存均为特殊用途，保留即可*/
			return AOS_RESERVED;
		case EFI_LOADER_DATA:
			/*该类型大概率是提前申请的模块与数据区域，但需要额外分辨*/
			return AOS_LOADER_DATA;
		case EFI_LOADER_CODE:
		case EFI_BOOT_SERVICES_CODE:
		case EFI_BOOT_SERVICES_DATA:
		case EFI_CONVENTIONAL_MEMORY:
			/*UEFI中认为可支配的内存*/
			return AOS_AVAILABLE;
		case EFI_RUNTIME_SERVICES_CODE:
			/*UEFI运行时的代码*/
			return AOS_FIRMWARE_CODE;
		case EFI_RUNTIME_SERVICES_DATA:
			/*UEFI运行时的数据*/
			return AOS_FIRMWARE_DATA;
		case EFI_ACPI_RECLAIM_MEMORY:
			/*ACPI表的内容*/
			return AOS_ACPI_TABLE;
		case EFI_ACPI_MEMORY_NVS:
			/*ACPI NVS保留内存*/
			return AOS_ACPI_NVS;
		case EFI_MEMORY_MAPPED_IO:
		case EFI_MEMORY_MAPPED_IO_PORT_SPACE:
			/*直接标记的内存映射端口*/
			return AOS_MMIO;
	}
}

/*
 * 申请一个结点。
 *
 * @param base	内存基址。
 * @param pages	内存页数。
 * @param type	内存类型。
 * @param pid	所属处理器编号。
 *
 * @return 结点句柄，申请失败返回未定义。
 */
static handle boot_pmm_node_alloc(const uintn base,const uintn pages,const uint16 type,const uint32 pid)
{
	handle result=boot_bitmap_pool_alloc(buddy.pool);
	if(result==HANDLE_UNDEFINED)
	{
		return HANDLE_UNDEFINED;
	}
	physical_page_frame* content=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,result);
	content->node.color=TREE_RED;
	content->node.left=HANDLE_UNDEFINED;
	content->node.right=HANDLE_UNDEFINED;
	content->node.parent=HANDLE_UNDEFINED;
	content->base=base;
	content->pages=pages;
	page_frame_set_type(content,type);
	page_frame_set_pid(content,pid);

	return result;
}

/*
 * 对结点进行归并。将低层级块归并到高层级，仅适用于伙伴链表的内存块。
 *
 * @param order 内存块阶数，不大于51。
 *
 * @return 无返回值。
 */
static void boot_pmm_buddy_merge(const uintn order)
{
	if(order>=51||buddy.blocks[order]==HANDLE_UNDEFINED)
	{
		return;
	}

	/*下一级分页基址的标记*/
	uintn mask=((uintn)1<<(order+13))-1;

	handle head=boot_tree_head(buddy.pool,buddy.blocks[order]);
	physical_page_frame* headc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head);
	handle next=boot_tree_next(buddy.pool,head);

	while(next!=HANDLE_UNDEFINED)
	{
		physical_page_frame* nextc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,next);

		if(!(headc->base&mask))
		{
			if(nextc->base==headc->base+(headc->pages<<12))
			{
				handle nodea=head;
				handle nodeb=next;

				head=boot_tree_next(buddy.pool,next);
				next=boot_tree_next(buddy.pool,head);
				boot_tree_remove(buddy.pool,&buddy.blocks[order],nodea);
				boot_tree_remove(buddy.pool,&buddy.blocks[order],nodeb);

				/*保留a结点，将其重新初始化*/
				headc->node.color=TREE_RED;
				headc->pages=headc->pages<<1;
				boot_tree_insert(buddy.pool,&buddy.blocks[order+1],boot_pmm_compare,nodea);
				boot_bitmap_pool_free(buddy.pool,nodeb);

				/*保守策略，每一次线性修改都需要重新读取开始结点，*/
				head=boot_tree_head(buddy.pool,buddy.blocks[order]);
				if(head==HANDLE_UNDEFINED)
				{
					return;
				}
				else
				{
					headc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head);
					next=boot_tree_next(buddy.pool,head);
					continue;
				}
			}
		}

		head=next;
		headc=nextc;
		next=boot_tree_next(buddy.pool,next);
	}
}

/*
 * 在伙伴系统中释放空闲内存空间。
 *
 * @param base	内存基址。
 * @param pages	内存页数。
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_pmm_buddy_free(uintn base,uintn pages)
{
	while(pages>0)
	{
		/*获取当前基址对应的最大可能对齐位数，推测对应块阶数，对0认为是sizeof(uintn)*8-1，x64下应为63*/
		uint8 mask=__builtin_ctzg(base,(int)sizeof(uintn)*8-1);
		uint8 order=mask-12;
		uintn page=(uintn)1<<order;

		/*推测页数大于实际页数时减少块阶数*/
		while(pages<page)
		{
			order--;
			page=(uintn)1<<order;
		}

		handle node=boot_pmm_node_alloc(base,page,AOS_AVAILABLE,0);
		if(node==HANDLE_UNDEFINED||!boot_tree_insert(buddy.pool,&buddy.blocks[order],boot_pmm_compare,node))
		{
			/*按检查上仅可能没有空闲结点时失败*/
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
 * 对结点进行切割。将高层级块切割到低层级，仅适用于伙伴链表的内存块。
 *
 * @param order	内存块阶数，大于0且小于52。
 * @param node	被切割结点句柄。
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_pmm_buddy_spilt(const uintn order,const handle node)
{
	if(order==0||order>51||node==HANDLE_UNDEFINED||!boot_bitmap_pool_require(buddy.pool,1))
	{
		return false;
	}

	physical_page_frame* nodec=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node);
	boot_tree_remove(buddy.pool,&buddy.blocks[order],node);
	
	nodec->pages=nodec->pages>>1;
	nodec->node.color=TREE_RED;
	handle next=boot_pmm_node_alloc(nodec->base+(nodec->pages<<12),nodec->pages,AOS_AVAILABLE,0);

	/*理论上不会报错*/
	return boot_tree_insert(buddy.pool,&buddy.blocks[order-1],boot_pmm_compare,node)&&boot_tree_insert(buddy.pool,&buddy.blocks[order-1],boot_pmm_compare,next);
}

/*
 * 物理页框地址定位。用于已使用树遍历。
 *
 * @param left	左页框。
 * @param right	右内存地址。
 * 
 * @return -1区间小于地址，0区间包含地址，1区间大于地址。
 */
static int boot_pmm_position(const handle left,const handle right)
{
	const physical_page_frame* node=(physical_page_frame*)left;
	const uintn addr=(uintn)right;

	if(addr<node->base)
	{
		return 1;
	}
	else if(addr<=(node->base-1+(node->pages<<12))) 
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
 * @param order	  内存块阶数。
 * @param pages	  内存页数。
 * @param type	  内存类型。
 * @param pid	  所属处理器编号。
 *
 * @return 已分配地址，或未定义。
 */
static uintn boot_pmm_record(const physical_page_frame* content,const handle node,const uintn order,const uintn pages,const memory_type type,const uint32 pid)
{
	handle rnode=boot_pmm_node_alloc(content->base,pages,type,pid);

	if(rnode==HANDLE_UNDEFINED||!boot_tree_insert(buddy.pool,&used[PMMS_USED_ALLOCATED],boot_pmm_compare,rnode))
	{
		return HANDLE_UNDEFINED;
	}
	uintn rbase=content->base;

	/*释放原有结点和空闲内存*/
	uintn fbase=content->base+(pages<<12);
	uintn fpage=content->pages-pages;
	boot_tree_remove(buddy.pool,&buddy.blocks[order],node);
	boot_bitmap_pool_free(buddy.pool,node);
	if(fpage>0)
	{
		boot_pmm_buddy_free(fbase,fpage);
	}
	return rbase;
}

/*
 * 在伙伴系统中申请所需地址范围内，尽可能最大地址的内存。
 *
 * @param min	最小地址边界。
 * @param max	最大地址边界。
 * @param pages	申请页数。
 * @param type	申请内存类型。
 * @param pid	所属处理器编号。
 *
 * @return 申请的地址，失败返回未定义。
 */
static uintn boot_pmm_buddy_alloc_max(const uintn min,const uintn max,const uintn pages,const memory_type type,const uint32 pid)
{
	/*计算阶数，先通过最高位估计阶数，如果不足则增大阶数*/
	uintn order=((sizeof(uintn)<<3)-1-__builtin_clzg(pages));
	if(((uintn)1<<order)<pages)
	{
		order++;
	}

	if(max-min<((uintn)1<<order)-1)
	{
		/*用于优先保证申请内存的对齐，允许一部分超出的奢侈*/
		return HANDLE_UNDEFINED;
	}

	uintn gindex=order;
	while(gindex<52)
	{
		handle node=boot_tree_tail(buddy.pool,buddy.blocks[gindex]);
		
		if(node==HANDLE_UNDEFINED)
		{
			/*情况1：该级没有空闲内存块*/
			gindex++;
			continue;
		}

		while(node!=HANDLE_UNDEFINED)
		{
			physical_page_frame* nodec=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node);
			int presult=boot_pmm_position((handle)nodec,max);

			if(presult==1)
			{
				/*情况2：结点大于上限*/
				node=boot_tree_previous(buddy.pool,node);
				if(node==HANDLE_UNDEFINED)
				{
					gindex++;
					break;
				}
				else
				{
					continue;
				}
			}
			else if(presult==0)
			{
				/*情况3：结点包含最大值*/
				if(max-nodec->base>=(pages<<12)-1)
				{
					/*足够*/
					if(gindex==order)
					{
						/*直接分配*/
						return boot_pmm_record(nodec,node,gindex,pages,type,pid);
					}
					else
					{
						/*内存切割*/
						if(boot_pmm_buddy_spilt(gindex,node))
						{
							gindex--;
							break;
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
				}
				else
				{
					/*不足，直接下一个*/
					node=boot_tree_previous(buddy.pool,node);
					if(node==HANDLE_UNDEFINED)
					{
						gindex++;
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				/*结点小于上限，比较下限*/
				presult=boot_pmm_position((handle)nodec,min);

				if(presult==-1)
				{
					/*情况4：结点小于下限*/
					gindex++;
					break;
				}
				else if(presult==1)
				{
					/*情况5：结点大于下限*/
					if(gindex==order)
					{
						return boot_pmm_record(nodec,node,gindex,pages,type,pid);
					}
					else
					{
						if(boot_pmm_buddy_spilt(gindex,node))
						{
							gindex--;
							break;
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
				}
				else
				{
					/*情况6：结点包含下限*/
					if(gindex==order)
					{
						/*强制要求下限对齐*/
						if(min==nodec->base)
						{
							return boot_pmm_record(nodec,node,gindex,pages,type,pid);
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
					else
					{
						if(boot_pmm_buddy_spilt(gindex,node))
						{
							gindex--;
							break;
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
				}
			}
		}
	}
	return HANDLE_UNDEFINED;
}

/*
 * 在伙伴系统中申请所需地址范围内，尽可能最小地址的内存。
 *
 * @param min	最小地址边界。
 * @param max	最大地址边界。
 * @param pages	申请页数。
 * @param type	申请内存类型。
 * @param pid	所属处理器编号。
 *
 * @return 申请的地址，失败返回未定义。
 */
static uintn boot_pmm_buddy_alloc_min(const uintn min,const uintn max,const uintn pages,const memory_type type,const uint32 pid)
{
	/*计算阶数，先通过最高位估计阶数，如果不足则增大阶数*/
	uintn order=((sizeof(uintn)<<3)-1-__builtin_clzg(pages));
	if(((uintn)1<<order)<pages)
	{
		order++;
	}

	if(max-min<((uintn)1<<order)-1)
	{
		/*用于优先保证申请内存的对齐，允许一部分超出的奢侈*/
		return HANDLE_UNDEFINED;
	}

	uintn gindex=order;
	while(gindex<52)
	{
		handle node=boot_tree_head(buddy.pool,buddy.blocks[gindex]);

		if(node==HANDLE_UNDEFINED)
		{
			/*情况1：该级没有空闲内存块*/
			gindex++;
			continue;
		}

		while(node!=HANDLE_UNDEFINED)
		{
			physical_page_frame* nodec=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node);
			int presult=boot_pmm_position((handle)nodec,min);

			if(presult==-1)
			{
				/*情况2：结点小于下限*/
				node=boot_tree_next(buddy.pool,node);
				if(node==HANDLE_UNDEFINED)
				{
					gindex++;
					break;
				}
				else
				{
					continue;
				}
			}
			else if(presult==0)
			{
				/*情况3：结点包含下限*/
				if(gindex==order)
				{
					if(min==nodec->base)
					{
						return boot_pmm_record(nodec,node,gindex,pages,type,pid);
					}
					else
					{
						node=boot_tree_next(buddy.pool,node);
						if(node==HANDLE_UNDEFINED)
						{
							gindex++;
							break;
						}
						else
						{
							continue;
						}
					}
				}
				else
				{
					if(boot_pmm_buddy_spilt(gindex,node))
					{
						gindex--;
						break;
					}
					else
					{
						return HANDLE_UNDEFINED;
					}
				}
			}
			else
			{
				/*结点大于下限*/
				presult=boot_pmm_position((handle)nodec,max);

				if(presult==1)
				{
					/*情况4：结点大于上限*/
					gindex++;
					break;
				}
				else if(presult==-1)
				{
					/*情况5：结点小于上限*/
					if(gindex==order)
					{
						return boot_pmm_record(nodec,node,gindex,pages,type,pid);
					}
					else
					{
						if(boot_pmm_buddy_spilt(gindex,node))
						{
							gindex--;
							break;
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
				}
				else
				{
					/*情况6：结点包含上限*/
					if(gindex==order)
					{
						if(max-nodec->base>=(pages<<12)-1)
						{
							return boot_pmm_record(nodec,node,gindex,pages,type,pid);
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
					else
					{
						if(boot_pmm_buddy_spilt(gindex,node))
						{
							gindex--;
							break;
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
				}
			}
		}
	}
	return HANDLE_UNDEFINED;
}

/*
 * 物理内存按模式选择申请。其申请范围为闭区间。
 * 
 * @param mode	申请模式。
 * @param min	申请范围下限。
 * @param max	申请范围上限。
 * @param pages 申请页数。
 * @param type	申请类型。
 * @param pid	所属处理器编号。
 *
 * @return 对应内存基址，失败返回未定义。 
 */
static uintn boot_pmm_alloc_switch(const malloc_mode mode,const uintn min,const uintn max,const uintn pages,const memory_type type,const uint32 pid)
{
	switch(mode)
	{
		case MALLOC_MIN:
			return boot_pmm_buddy_alloc_min(min,SIZE_MAX,pages,type,pid);
		case MALLOC_MAX:
			return boot_pmm_buddy_alloc_max(0,max,pages,type,pid);
		case MALLOC_RANGE:
			return boot_pmm_buddy_alloc_max(min,max,pages,type,pid);
		case MALLOC_ANY:
			return boot_pmm_buddy_alloc_max(0,SIZE_MAX,pages,type,pid);
		default:
			return HANDLE_UNDEFINED;
	}
}

/*
 * 物理内存申请。其申请范围为闭区间。
 * 
 * @param mode	申请模式。
 * @param min	申请范围下限。
 * @param max	申请范围上限。
 * @param pages 申请页数。
 * @param type	申请类型。
 * @param pid	所属处理器编号。
 *
 * @return 对应内存基址，失败返回未定义。 
 */
extern uintn boot_pmm_alloc(const malloc_mode mode,const uintn min,const uintn max,const uintn pages,const memory_type type,const uint32 pid)
{
	if(pages==0||min>=max||type>=AOS_MAX_MEMORY_TYPE)
	{
		return HANDLE_UNDEFINED;
	}

	if(!boot_bitmap_pool_require(buddy.pool,PMMS_POOL_RESERVED))
	{
		/*如果池子剩余结点不足，申请新的内存1MB*/
		return HANDLE_UNDEFINED;
	}

	uintn result=boot_pmm_alloc_switch(mode,min,max,pages,type,pid);

	if(result==HANDLE_UNDEFINED)
	{
		/*第一次失败，重整内存*/
		for(uintn order=0;order<51;order++)
		{
			boot_pmm_buddy_merge(order);
		}

		return boot_pmm_alloc_switch(mode,min,max,pages,type,pid);
	}
	else 
	{
		return result;
	}
}

/*
 * 物理页框地址查找。用于已分配内存块的检查。
 *
 * @param addr 内存地址。
 * 
 * @return 返回找到结点，或未定义。
 */
static handle boot_pmm_find(const uintn addr)
{
	return boot_tree_find(buddy.pool,&used[PMMS_USED_ALLOCATED],boot_pmm_position,addr);
}

/*
 * 物理内存释放。
 * 
 * @param addr 需要释放区域内的地址。
 *
 * @return 成功返回真，出错返回假。 
 */
extern bool boot_pmm_free(const uintn addr)
{
	handle node=boot_pmm_find(addr);

	if(node!=HANDLE_UNDEFINED)
	{
		physical_page_frame* content=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node);
		uintn base=content->base;
		uintn pages=content->pages;
		boot_tree_remove(buddy.pool,&used[PMMS_USED_ALLOCATED],node);
		boot_bitmap_pool_free(buddy.pool,node);
		return boot_pmm_buddy_free(base,pages);
	}

	return true;
}

/*
 * 物理内存管理系统(PMMS)初始化。
 *
 * @param global 全局函数表。
 * @param params 启动核参数。
 * 
 * @return 无返回值。
 */
extern void boot_pmm_init(const boot_function_table* restrict global,const boot_params* restrict params)
{
	/*标记预加载内存，先记录内存块，后记录模块*/
	static boot_block loader_data[BOOT_BLOCK_COUNT+BOOT_MODULE_COUNT];
	for(uintn index=0;index<BOOT_BLOCK_COUNT;index++)
	{
		loader_data[index].base=params->blocks[index].base;
		loader_data[index].pages=params->blocks[index].pages;
	}
	for(uintn index=0;index<BOOT_MODULE_COUNT;index++)
	{
		loader_data[BOOT_BLOCK_COUNT+index].base=params->modules[index].base;
		loader_data[BOOT_BLOCK_COUNT+index].pages=params->modules[index].pages;
	}

	/*对地址从小到大排序*/
	for(uintn index=0;index<BOOT_BLOCK_COUNT+BOOT_MODULE_COUNT-1;index++)
	{
		boot_block data;
		for(uintn ptr=index+1;ptr<BOOT_BLOCK_COUNT+BOOT_MODULE_COUNT;ptr++)
		{
			if(loader_data[index].base>loader_data[ptr].base)
			{
				data.base=loader_data[index].base;
				data.pages=loader_data[index].pages;
				loader_data[index].base=loader_data[ptr].base;
				loader_data[index].pages=loader_data[ptr].pages;
				loader_data[ptr].base=data.base;
				loader_data[ptr].pages=data.pages;
			}
		}
	}

	/*初始化位映射池表头*/
	boot_pmm_bitmap_pool_init(global,params);

	/*内存图转换*/
	efi_memory_descriptor* dsc=params->env.memmap;
	uintn offset=params->env.entry_size;
	uintn end=params->env.memmap_length+(uintn)dsc;
	while((uintn)dsc<end)
	{
		memory_type type=boot_pmm_efi2aos(dsc->type);

		if(type==AOS_AVAILABLE)
		{
			if(dsc->physical_start<0x100000)
			{
				/*最低1MB区*/
				handle node=boot_pmm_node_alloc(dsc->physical_start,dsc->pages,AOS_LOWEST_MEMORY,0);
				boot_tree_insert(buddy.pool,&used[PMMS_USED_RESERVED],boot_pmm_compare,node);
			}
			else
			{
				/*自由内存区域*/
				boot_pmm_buddy_free(dsc->physical_start,dsc->pages);
			}
		}
		else if(type==AOS_LOADER_DATA)
		{
			/*预申请区检查*/
			uintn start=dsc->physical_start;
			uintn end=dsc->physical_start+(dsc->pages<<12)-1;

			for(uintn index=0;index<BOOT_BLOCK_COUNT+BOOT_MODULE_COUNT;index++)
			{
				uintn block_start=loader_data[index].base;
				uintn block_end=block_start+(loader_data[index].pages<<12)-1;

				if(start<=block_start&&block_end<=end)
				{
					if(start!=block_start)
					{
						boot_pmm_buddy_free(start,(block_start-start)>>12);
						start=block_start;
					}
					/*预申请区*/
					start=block_end+1;
					handle node=boot_pmm_node_alloc(loader_data[index].base,loader_data[index].pages,AOS_LOADER_DATA,0);
					boot_tree_insert(buddy.pool,&used[PMMS_USED_ALLOCATED],boot_pmm_compare,node);
				}
			}
			/*自由内存区域*/
			if(start<end)
			{
				boot_pmm_buddy_free(start,(end+1-start)>>12);
			}
		}
		else
		{
			/*其他内存资源，MMIO也算*/
			handle node=boot_pmm_node_alloc(dsc->physical_start,dsc->pages,type,0);
			boot_tree_insert(buddy.pool,&used[PMMS_USED_RESERVED],boot_pmm_compare,node);
		}

		dsc=(efi_memory_descriptor*)((uintn)dsc+offset);
	}

	/*整合最低1MB区域*/
	uintn head=boot_tree_head(buddy.pool,used[PMMS_USED_RESERVED]);
	physical_page_frame* headc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head);
	uintn next=boot_tree_next(buddy.pool,head);

	while(next!=HANDLE_UNDEFINED&&page_frame_get_type(headc)==AOS_LOWEST_MEMORY)
	{
		physical_page_frame* nextc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,next);

		if(page_frame_get_type(nextc)==AOS_LOWEST_MEMORY&&nextc->base==headc->base+(headc->pages<<12))
		{
			boot_tree_remove(buddy.pool,&used[PMMS_USED_RESERVED],head);
			boot_tree_remove(buddy.pool,&used[PMMS_USED_RESERVED],next);

			/*保留前结点，将其重新初始化，释放后结点*/
			headc->node.color=TREE_RED;
			headc->pages=headc->pages+nextc->pages;
			boot_tree_insert(buddy.pool,&used[PMMS_USED_RESERVED],boot_pmm_compare,head);
			boot_bitmap_pool_free(buddy.pool,next);
			next=boot_tree_next(buddy.pool,head);
			continue;
		}

		head=next;
		headc=nextc;
		next=boot_tree_next(buddy.pool,next);
	}

	/*整合各阶内存块*/
	for(uintn order=0;order<51;order++)
	{
		boot_pmm_buddy_merge(order);
	}
}
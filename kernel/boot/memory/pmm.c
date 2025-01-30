/*
 * 物理内存管理系统(PMMS)。
 * @date 2025-01-03
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/pmm.h"

/*物理内存管理伙伴系统*/
static buddy_list buddy={
	.pool=(void*)HANDLE_UNDEFINED,
	.block={[0 ... 51]=HANDLE_UNDEFINED}
};

/*
 * 物理内存位映射池初始化。
 *
 * @param global 全局函数表。
 * @param params 启动核参数。
 *
 * @return 无返回值。
 */
static void boot_pmm_bitmap_pool_init(const boot_function_table* restrict global,boot_params* restrict params)
{
	uintn base=(((uintn)params->current_pointer>>12)+((uintn)params->current_pointer&0xFFF?1:0))<<12;

	if((uintn)params->pool+params->pool_length-base<BOOT_PMP_SIZE)
	{
		/*可用内存不足，可能是参数设置问题，也有可能是预设内存池大小问题*/
		DEBUG_START
		/*错误：引导器预设内存池空间不足以分配该数目的物理页框结点，请尝试重新配置BOOT_PHYSICAL_MEMORY_POOL。系统因此将在此处关机。*/
		#define BOOT_PM_ERROR_MSG "Error:Bootloader preset memory pool space is insufficient to allocate that number of physical page frame nodes.Try reconfiguring BOOT_PHYSICAL_MEMORY_POOL.The system will therefore shut down here.\n"
		global->util->writeport(BOOT_PM_ERROR_MSG,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(BOOT_PM_ERROR_MSG)-1);
		DEBUG_END

		global->util->ms_call_4(params->runtime->reset_system,EFI_RESET_SHUTDOWN,EFI_OUT_OF_RESOURCES,0,0);
		__builtin_unreachable();
	}

	params->current_pointer=(void*)(base+BOOT_PMP_SIZE);
	boot_bitmap_pool_init(global,(void*)base,BOOT_PHYSICAL_MEMORY_POOL,sizeof(physical_page_frame));
	buddy.pool=(bitmap_pool*)base;
}

/*
 * 物理页框区间比较函数。用于红黑树插入。
 *
 * @param left	左页框句柄。
 * @param right	右页框句柄。
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
		/*如果出现未定义这里将来要申请新内存池*/
		return result;
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
 * @param block	内存块索引，不大于51。
 *
 * @return 无返回值。
 */
static void boot_pmm_buddy_merge(const uintn block)
{
	if(block>=51||buddy.block[block]==HANDLE_UNDEFINED)
	{
		return;
	}

	/*下一级分页基址的标记*/
	uintn mask=((uintn)1<<(block+13))-1;

	uintn head=boot_tree_head(buddy.pool,buddy.block[block]);
	physical_page_frame* headc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head);
	uintn next=boot_tree_next(buddy.pool,head);

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
				boot_tree_remove(buddy.pool,&buddy.block[block],nodea);
				boot_tree_remove(buddy.pool,&buddy.block[block],nodeb);

				/*保留a结点，将其重新初始化*/
				headc->node.color=TREE_RED;
				headc->pages=headc->pages<<1;
				boot_tree_insert(buddy.pool,&buddy.block[block+1],boot_pmm_compare,nodea);
				boot_bitmap_pool_free(buddy.pool,nodeb);

				if(next==HANDLE_UNDEFINED)
				{
					/*仅剩0或1个结点，无需再继续搜索*/
					return;
				}
				else
				{
					headc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head);
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
		/*获取当前基址对应的最大可能对齐位数，推测对应块索引，对0认为是sizeof(uintn)*8-1，x64下应为63*/
		uint8 mask=__builtin_ctzg(base,(int)sizeof(uintn)*8-1);
		uint8 index=mask-12;
		uintn page=(uintn)1<<index;

		/*推测页数大于实际页数时减少块索引*/
		while(pages<page)
		{
			index--;
			page=(uintn)1<<index;
		}

		handle node=boot_pmm_node_alloc(base,page,AOS_AVAILABLE,0);
		if(node==HANDLE_UNDEFINED||!boot_tree_insert(buddy.pool,&buddy.block[index],boot_pmm_compare,node))
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
 * @param block	内存块索引，大于0。
 * @param node	被切割结点句柄。
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_pmm_buddy_spilt(const uintn block,const handle node)
{
	if(block==0||block>51||node==HANDLE_UNDEFINED||!boot_bitmap_pool_require(buddy.pool,1))
	{
		return false;
	}

	physical_page_frame* nodec=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node);
	boot_tree_remove(buddy.pool,&buddy.block[block],node);
	
	nodec->pages=nodec->pages>>1;
	nodec->node.color=TREE_RED;
	handle next=boot_pmm_node_alloc(nodec->base+(nodec->pages<<12),nodec->pages<<12,AOS_AVAILABLE,0);

	/*理论上不会报错*/
	return boot_tree_insert(buddy.pool,&buddy.block[block-1],boot_pmm_compare,node)&&boot_tree_insert(buddy.pool,&buddy.block[block-1],boot_pmm_compare,next);
}

/*
 * 物理页框地址定位函数。用于红黑树线性遍历。
 *
 * @param node 结点。
 * @param addr 内存地址。
 * 
 * @return -1区间小于地址，0区间包含地址，1区间大于地址。
 */
static int boot_pmm_position(const physical_page_frame* node,const uintn addr)
{
	if(addr<node->base)
	{
		return 1;
	}
	else if(addr<(node->base+(node->pages<<12))) 
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

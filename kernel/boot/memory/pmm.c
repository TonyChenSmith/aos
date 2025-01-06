/*
 * 物理内存管理系统。
 * @date 2025-01-03
 */
#include "include/pmm.h"
#include "basic_type.h"
#include "include/boot_tree.h"
#include "memory/page_frame.h"
#include "util/tree.h"

/*伙伴系统*/
static buddy_list buddy={
	.pool=(void*)HANDLE_UNDEFINED,
	.block={[0 ... 51]=HANDLE_UNDEFINED}
};

/*
 * 物理内存位映射池初始化。
 *
 * @param param 启动参数结构。
 * @param bbft	基础模块函数表。
 *
 * @return 无返回值。
 */
static void boot_pmm_bitmap_pool_init(boot_params* restrict param,const boot_base_functions* bbft)
{
	uintn base=(((uintn)param->current_pointer>>12)+((uintn)param->current_pointer&0xFFF?1:0))<<12;
	if((uintn)param->pool+param->pool_length-base<BOOT_PMP_SIZE)
	{
		/*可用内存不足，可能是参数设置问题，也有可能是预设内存池大小问题*/
		DEBUG_START
		/*错误：引导器预设内存池空间不足以分配该数目的物理页框结点，请尝试重新配置BOOT_PHYSICAL_MEMORY_POOL。系统因此将在此处关机。*/
		#define BOOT_PM_ERROR_MSG "Error:Bootloader preset memory pool space is insufficient to allocate that number of physical page frame nodes.Try reconfiguring BOOT_PHYSICAL_MEMORY_POOL.The system will therefore shut down here.\n"
		bbft->boot_writeport(BOOT_PM_ERROR_MSG,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(BOOT_PM_ERROR_MSG)-1);
		DEBUG_END
		bbft->boot_ms_call_4(param->runtime->reset_system,EFI_RESET_SHUTDOWN,EFI_OUT_OF_RESOURCES,0,0);
		__builtin_unreachable();
	}
	param->current_pointer=(void*)(base+BOOT_PMP_SIZE);
	boot_bitmap_pool_init((void*)base,BOOT_PHYSICAL_MEMORY_POOL,sizeof(physical_page_frame),bbft);
	buddy.pool=(bitmap_pool*)base;
}

/*
 * 物理页框区间比较。
 *
 * @param left	左页框句柄。
 * @param right	右页框句柄。
 * 
 * @return -1小于，0等于或左覆盖右，1大于，2左右交错，3右左交错，4右覆盖左。在系统中原则上不出现后三种情况。
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
			return AOS_RESERVED;
		case EFI_LOADER_CODE:
		case EFI_LOADER_DATA:
			return AOS_LOADER_DATA;
		case EFI_BOOT_SERVICES_CODE:
		case EFI_BOOT_SERVICES_DATA:
		case EFI_CONVENTIONAL_MEMORY:
			return AOS_AVAILABLE;
		case EFI_RUNTIME_SERVICES_CODE:
			return AOS_FIRMWARE_CODE;
		case EFI_RUNTIME_SERVICES_DATA:
			return AOS_FIRMWARE_DATA;
		case EFI_ACPI_RECLAIM_MEMORY:
			return AOS_ACPI_TABLE;
		case EFI_ACPI_MEMORY_NVS:
			return AOS_ACPI_NVS;
		case EFI_MEMORY_MAPPED_IO:
		case EFI_MEMORY_MAPPED_IO_PORT_SPACE:
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
				headc->node.color=TREE_RED;
				headc->node.parent=HANDLE_UNDEFINED;
				headc->node.left=HANDLE_UNDEFINED;
				headc->node.right=HANDLE_UNDEFINED;
				headc->pages=headc->pages<<1;
				boot_tree_insert(buddy.pool,&buddy.block[block+1],boot_pmm_compare,nodea);
				boot_bitmap_pool_free(buddy.pool,nodeb);
				if(next==HANDLE_UNDEFINED)
				{
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
 * 在伙伴系统中释放空闲内存。
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
		uint8 mask=__builtin_ctzg(base,(int)sizeof(uintn)*8-1);
		uint8 index=mask-12;
		uintn page=(uintn)1<<index;
		while(pages<page)
		{
			index--;
			page=(uintn)1<<index;
		}
		handle node=boot_pmm_node_alloc(base,page,AOS_AVAILABLE,0);
		if(node==HANDLE_UNDEFINED||!boot_tree_insert(buddy.pool,&buddy.block[index],boot_pmm_compare,node))
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
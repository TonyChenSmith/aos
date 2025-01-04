/*
 * 物理内存伙伴系统。
 * @date 2024-12-15
 */
#include "include/pmm.h"

/*伙伴系统*/
static buddy_list buddy={
	.pool=(void*)HANDLE_UNDEFINED,
	.block={[0 ... 51]={HANDLE_UNDEFINED,HANDLE_UNDEFINED}}
};

/*记录链表，0为已分配链表，1为映射链表，2为其他记录*/
static linked_list record_list[3]={[0 ... 2]={HANDLE_UNDEFINED,HANDLE_UNDEFINED}};

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
 * 物理页框结点清空。
 *
 * @param node 结点。
 * 
 * @return 无返回值。
 */
static void boot_pmm_clear(const handle node)
{
	__builtin_memset_inline(boot_bitmap_pool_content(buddy.pool,node,sizeof(physical_page_frame)),0,sizeof(physical_page_frame));
}

/*
 * 物理页框区间比较。
 *
 * @param left	左页框。
 * @param right	右页框。
 * 
 * @return -1小于，0等于或左覆盖右，1大于，2左右交错，3右左交错，4右覆盖左。
 */
static int32 boot_pmm_compare(const physical_page_frame* left,const physical_page_frame*right)
{
	uintn la=left->base;
	uintn lb=la-1+(left->pages<<12);
	uintn ra=right->base;
	uintn rb=ra-1+(right->pages<<12);
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
static bool boot_pmm_list_add(linked_list* restrict list,const handle node)
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
		physical_page_frame* left;
		physical_page_frame* right;
		right=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(physical_page_frame));
		while(head!=HANDLE_UNDEFINED)
		{
			left=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(physical_page_frame));
			switch(boot_pmm_compare(left,right))
			{
				case -1:
					/*小于，到下一结点*/
					head=left->next;
					continue;
				case 1:
					/*大于，添加结点*/
					if(left->prev!=HANDLE_UNDEFINED)
					{
						physical_page_frame* pc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,left->prev,sizeof(physical_page_frame));
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
		left=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,list->tail,sizeof(physical_page_frame));
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
static bool boot_pmm_list_remove(linked_list* restrict list,const handle node)
{
	handle head=list->head;
	while(head!=HANDLE_UNDEFINED)
	{
		physical_page_frame* hc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(physical_page_frame));
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
					hc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->next,sizeof(physical_page_frame));
					hc->prev=HANDLE_UNDEFINED;
				}
			}
			else if(hc->next==HANDLE_UNDEFINED) 
			{
				/*尾结点*/
				list->tail=hc->prev;
				hc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->prev,sizeof(physical_page_frame));
				hc->next=HANDLE_UNDEFINED;
			}
			else
			{
				/*中间结点*/
				physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->next,sizeof(physical_page_frame));
				physical_page_frame* pc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->prev,sizeof(physical_page_frame));
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
 *
 * @return 结点句柄，申请失败返回未定义。
 */
static handle boot_pmm_node_alloc(const uintn base,const uintn pages,const uint16 type)
{
	handle result=boot_bitmap_pool_alloc(buddy.pool);
	if(result==HANDLE_UNDEFINED)
	{
		/*如果出现未定义这里将来要申请新内存池*/
		return result;
	}
	physical_page_frame* content=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,result,sizeof(physical_page_frame));
	content->prev=HANDLE_UNDEFINED;
	content->next=HANDLE_UNDEFINED;
	content->base=base;
	content->pages=pages;
	content->type=type;
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
	if(block>=51||buddy.block[block].head==HANDLE_UNDEFINED)
	{
		return;
	}
	uintn mask=((uintn)1<<(block+13))-1;
	uintn head=buddy.block[block].head;
	physical_page_frame* hc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(physical_page_frame));
	while(hc->next!=HANDLE_UNDEFINED)
	{
		physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,hc->next,sizeof(physical_page_frame));
		if(!(hc->base&mask))
		{
			if(nc->base==hc->base+(hc->pages<<12))
			{
				uintn tbase=hc->base;
				uintn tpage=hc->pages<<1;
				handle ra=head;
				handle rb=hc->next;
				boot_pmm_list_remove(&buddy.block[block],ra);
				boot_pmm_list_remove(&buddy.block[block],rb);
				head=boot_pmm_node_alloc(tbase,tpage,AOS_AVAILABLE);
				boot_pmm_list_add(&buddy.block[block+1],head);
				head=buddy.block[block].head;
				if(head==HANDLE_UNDEFINED)
				{
					return;
				}
				else
				{
					hc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(physical_page_frame));
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
		handle node=boot_pmm_node_alloc(base,page,AOS_AVAILABLE);
		if(node==HANDLE_UNDEFINED||!boot_pmm_list_add(&buddy.block[index],node))
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
 *
 * @return 成功返回真，失败返回假。
 */
static bool boot_pmm_buddy_spilt(const uintn block,const handle node)
{
	if(block==0||block>51||node==HANDLE_UNDEFINED)
	{
		return false;
	}
	physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(physical_page_frame));
	uintn tbase=nc->base;
	uintn tpage=nc->pages>>1;
	boot_pmm_list_remove(&buddy.block[block],node);
	handle a=boot_pmm_node_alloc(tbase,tpage,AOS_AVAILABLE);
	handle b=boot_pmm_node_alloc(tbase+(tpage<<12),tpage,AOS_AVAILABLE);
	if(b==HANDLE_UNDEFINED)
	{
		nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,a,sizeof(physical_page_frame));
		nc->pages=tpage<<1;
		boot_pmm_list_add(&buddy.block[block],a);
		return false;
	}
	boot_pmm_list_add(&buddy.block[block-1],a);
	boot_pmm_list_add(&buddy.block[block-1],b);
	return true;
}

/*
 * 内存地址相对结点定位。结果为结点比较地址。
 *
 * @param node 结点。
 * @param addr 内存地址。
 * 
 * @return -1小于，0包含，1大于。
 */
static int32 boot_pmm_position(const physical_page_frame* node,const uintn addr)
{
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
 * @param block	  内存块链表。
 * @param pages	  内存页数。
 * @param type	  内存类型。
 *
 * @return 已分配地址，或未定义。
 */
static uintn boot_pmm_record(const physical_page_frame* content,const uintn node,const uintn block,const uintn pages,const memory_type type)
{
	handle rnode=boot_pmm_node_alloc(content->base,pages,type);
	if(rnode==HANDLE_UNDEFINED)
	{
		return HANDLE_UNDEFINED;
	}
	boot_pmm_list_add(&record_list[0],rnode);
	uintn rbase=content->base;
	uintn fbase=content->base+(pages<<12);
	uintn fpage=content->pages-pages;
	boot_pmm_list_remove(&buddy.block[block],node);
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
 *
 * @return 申请的地址，失败返回未定义。
 */
static uintn boot_pmm_buddy_alloc_max(const uintn min,const uintn max,const uintn pages,const memory_type type)
{
	uintn need=((sizeof(uintn)<<3)-1-__builtin_clzg(pages));
	if(((uintn)1<<need)<pages)
	{
		need++;
	}
	if(max-min<((uintn)1<<need)-1)
	{
		/*用于优先保证对齐，允许这一部分的奢侈*/
		return HANDLE_UNDEFINED;
	}
	uintn index=need;
	while(index<52)
	{
		handle node=buddy.block[index].tail;
		if(node==HANDLE_UNDEFINED)
		{
			index++;
			continue;
		}
		while(node!=HANDLE_UNDEFINED)
		{
			physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(physical_page_frame));
			int32 presult=boot_pmm_position(nc,max);
			if(presult==1)
			{
				/*结点大于上限*/
				node=nc->prev;
				if(node==HANDLE_UNDEFINED)
				{
					index++;
					break;
				}
				else
				{
					continue;
				}
			}
			else if(presult==0)
			{
				/*包含*/
				if(max-nc->base>=(pages<<12)-1)
				{
					/*足够*/
					if(index==need)
					{
						return boot_pmm_record(nc,node,index,pages,type);
					}
					else
					{
						if(boot_pmm_buddy_spilt(index,node))
						{
							index--;
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
					node=nc->prev;
					if(node==HANDLE_UNDEFINED)
					{
						index++;
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
				/*结点小于上限*/
				presult=boot_pmm_position(nc,min);
				if(presult==-1)
				{
					/*结点小于下限*/
					index++;
					break;
				}
				else if(presult==1)
				{
					/*结点大于上限*/
					if(index==need)
					{
						return boot_pmm_record(nc,node,index,pages,type);
					}
					else
					{
						if(boot_pmm_buddy_spilt(index,node))
						{
							index--;
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
					/*包含*/
					if(index==need)
					{
						if(min==nc->base)
						{
							return boot_pmm_record(nc,node,index,pages,type);
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
					else
					{
						if(boot_pmm_buddy_spilt(index,node))
						{
							index--;
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
 *
 * @return 申请的地址，失败返回未定义。
 */
static uintn boot_pmm_buddy_alloc_min(const uintn min,const uintn max,const uintn pages,const memory_type type)
{
	uintn need=((sizeof(uintn)<<3)-1-__builtin_clzg(pages));
	if(((uintn)1<<need)<pages)
	{
		need++;
	}
	if(max-min<((uintn)1<<need)-1)
	{
		/*用于优先保证对齐，允许这一部分的奢侈*/
		return HANDLE_UNDEFINED;
	}
	uintn index=need;
	while(index<52)
	{
		handle node=buddy.block[index].head;
		if(node==HANDLE_UNDEFINED)
		{
			index++;
			continue;
		}
		while(node!=HANDLE_UNDEFINED)
		{
			physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(physical_page_frame));
			int32 presult=boot_pmm_position(nc,min);
			if(presult==-1)
			{
				/*结点小于下限*/
				node=nc->next;
				if(node==HANDLE_UNDEFINED)
				{
					index++;
					break;
				}
				else
				{
					continue;
				}
			}
			else if(presult==0)
			{
				/*包含*/
				if(index==need)
				{
					if(min==nc->base)
					{
						return boot_pmm_record(nc,node,index,pages,type);
					}
					else
					{
						node=nc->next;
						if(node==HANDLE_UNDEFINED)
						{
							index++;
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
					if(boot_pmm_buddy_spilt(index,node))
					{
						index--;
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
				presult=boot_pmm_position(nc,max);
				if(presult==1)
				{
					/*结点大于上限*/
					index++;
					break;
				}
				else if(presult==-1)
				{
					/*结点小于上限*/
					if(index==need)
					{
						return boot_pmm_record(nc,node,index,pages,type);
					}
					else
					{
						if(boot_pmm_buddy_spilt(index,node))
						{
							index--;
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
					/*包含*/
					if(index==need)
					{
						if(max-nc->base>=(pages<<12))
						{
							return boot_pmm_record(nc,node,index,pages,type);
						}
						else
						{
							return HANDLE_UNDEFINED;
						}
					}
					else
					{
						if(boot_pmm_buddy_spilt(index,node))
						{
							index--;
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
 * 物理内存管理调试。
 *
 * @return 无返回值。
 */
static void boot_pmm_debug_dump(void)
{
	void plist(uintn list,uintn head,uintn tail);
	void pline(uintn index,uintn node,uintn start,uintn end,uintn amount,uintn type);
	for(uintn index=0;index<52;index++)
	{
		plist(index,buddy.block[index].head,buddy.block[index].tail);
		handle head=buddy.block[index].head;
		uintn i=0;
		while(head!=HANDLE_UNDEFINED)
		{
			physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(physical_page_frame));
			pline(i++,head,nc->base,nc->base+(nc->pages<<12)-1,nc->pages,nc->type);
			head=nc->next;
		}
	}
	for(uintn index=0;index<3;index++)
	{
		plist(index+52,record_list[index].head,record_list[index].tail);
		handle head=record_list[index].head;
		uintn i=0;
		while(head!=HANDLE_UNDEFINED)
		{
			physical_page_frame* nc=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,head,sizeof(physical_page_frame));
			pline(i++,head,nc->base,nc->base+(nc->pages<<12)-1,nc->pages,nc->type);
			head=nc->next;
		}
	}
}

/*
 * 物理内存管理初始化。
 *
 * @param param 启动参数结构。
 * @param bbft	基础模块函数表。
 *
 * @return 无返回值。
 */
extern void boot_pmm_init(boot_params* restrict param,const boot_base_functions* bbft)
{
	static uintn loader_data[BOOT_MODULE_COUNT+1][2];
	for(uintn index=0;index<BOOT_MODULE_COUNT;index++)
	{
		loader_data[index][0]=param->modules[index].base;
		loader_data[index][1]=param->modules[index].base+(param->modules[index].pages<<12);
	}
	loader_data[BOOT_MODULE_COUNT][0]=(uintn)param->pool;
	loader_data[BOOT_MODULE_COUNT][1]=(uintn)param->pool+param->pool_length;

	boot_pmm_bitmap_pool_init(param,bbft);

	efi_memory_descriptor* dsc=param->env.memmap;
	uintn offset=param->env.entry_size;
	uintn end=param->env.memmap_length+(uintn)dsc;
	while((uintn)dsc<end)
	{
		memory_type type=boot_pmm_efi2aos(dsc->type);
		if(type==AOS_AVAILABLE)
		{
			if(dsc->physical_start<0x100000)
			{
				/*最低1MB区*/
				handle node=boot_pmm_node_alloc(dsc->physical_start,dsc->pages,AOS_LOWEST_MEMORY);
				boot_pmm_list_add(&record_list[2],node);
			}
			else
			{
				/*自由内存区域*/
				boot_pmm_buddy_free(dsc->physical_start,dsc->pages);
			}
		}
		else if(type==AOS_LOADER_DATA)
		{
			/*预申请区*/
			uintn start=dsc->physical_start;
			uintn end=dsc->physical_start+(dsc->pages<<12);
			bool added=false;
			for(uintn i=0;i<BOOT_MODULE_COUNT+1;i++)
			{
				if(start<=loader_data[i][0]&&end>=loader_data[i][1])
				{
					if(i==0xFF)
					{
						/*预留给多核初始化模块aos.boot.mpinit*/
					}
					else
					{
						/*启动内核文件*/
						handle node=boot_pmm_node_alloc(dsc->physical_start,dsc->pages,AOS_LOADER_DATA);
						boot_pmm_list_add(&record_list[0],node);
					}
					added=true;
					break;
				}
			}
			if(!added)
			{
				/*自由内存区域*/
				boot_pmm_buddy_free(dsc->physical_start,dsc->pages);
			}
		}
		else
		{
			/*其他内存资源，MMIO也算*/
			handle node=boot_pmm_node_alloc(dsc->physical_start,dsc->pages,type);
			boot_pmm_list_add(&record_list[2],node);
		}
		dsc=(efi_memory_descriptor*)((uintn)dsc+offset);
	}
	
	for(uintn index=0;index<51;index++)
	{
		boot_pmm_buddy_merge(index);
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
 *
 * @return 对应内存基址，失败返回未定义。 
 */
extern uintn boot_pmm_alloc(const malloc_mode mode,const uintn min,const uintn max,const uintn pages,const memory_type type)
{
	if(pages==0||min>=max||type>=AOS_MAX_MEMORY_TYPE)
	{
		return HANDLE_UNDEFINED;
	}
	uintn result=HANDLE_UNDEFINED;
	switch(mode)
	{
		case MALLOC_MIN:
			result=boot_pmm_buddy_alloc_min(min,SIZE_MAX,pages,type);
			break;
		case MALLOC_MAX:
			result=boot_pmm_buddy_alloc_max(0,max,pages,type);
			break;
		case MALLOC_RANGE:
			result=boot_pmm_buddy_alloc_max(min,max,pages,type);
			break;
		case MALLOC_ANY:
			result=boot_pmm_buddy_alloc_max(0,SIZE_MAX,pages,type);
			break;
		default:
			return HANDLE_UNDEFINED;
	}
	if(result==HANDLE_UNDEFINED)
	{
		for(uintn index=0;index<51;index++)
		{
			boot_pmm_buddy_merge(index);
		}
		switch(mode)
		{
			case MALLOC_MIN:
				return boot_pmm_buddy_alloc_min(min,SIZE_MAX,pages,type);
			case MALLOC_MAX:
				return boot_pmm_buddy_alloc_max(0,max,pages,type);
			case MALLOC_RANGE:
				return boot_pmm_buddy_alloc_max(min,max,pages,type);
			case MALLOC_ANY:
				return boot_pmm_buddy_alloc_max(0,SIZE_MAX,pages,type);
			default:
				return HANDLE_UNDEFINED;
		}
	}
	else 
	{
		return result;
	}
}

/*
 * 物理内存释放。
 * 
 * @param addr 需要释放区域内的地址。
 *
 * @return 无返回值。 
 */
extern void boot_pmm_free(const uintn addr)
{
	handle node=record_list[0].head;
	while(node!=HANDLE_UNDEFINED)
	{
		physical_page_frame* content=(physical_page_frame*)boot_bitmap_pool_content(buddy.pool,node,sizeof(physical_page_frame));
		if(boot_pmm_position(content,addr)==0)
		{
			uintn base=content->base;
			uintn pages=content->pages;
			boot_pmm_list_remove(&record_list[0],node);
			boot_pmm_buddy_free(base,pages);
			return;
		}
		else
		{
			node=content->next;
		}
	}
}

/*
 * 物理内存管理内存申请释放测试。
 *
 * @return 无返回值。
 */
extern void boot_pmm_debug_test(void)
{
	extern void prints(const char* src);
	extern void print_num(uintn number);
	extern void print_hex(uintn number);
	prints("===Module aos.boot.memory Test===\ncurrent buddy:\n");
	boot_pmm_debug_dump();
	prints("===Test start===\n");
	uintn tp=1;
	for(uintn index=0;index<19;index++)
	{
		prints("test pages:");
		print_num(tp);
		prints("\nalloc max:[0x");
		print_hex(0);
		prints(",0x");
		print_hex(0x3FFFFFFF);
		prints("]=0x");
		uintn tb=boot_pmm_alloc(MALLOC_MAX,0,0x3FFFFFFF,tp,AOS_KERNEL_DATA);
		print_hex(tb);
		prints("\n");
		if(tb==HANDLE_UNDEFINED)
		{
			prints("alloc max fault.\n");
		}
		else
		{
			boot_pmm_free(tb);
		}
		prints("alloc min:[0x");
		print_hex(0x40000000);
		prints(",0x");
		print_hex(0xFFFFFFFF);
		prints("]=0x");
		tb=boot_pmm_alloc(MALLOC_MIN,0x40000000,0xFFFFFFFF,tp,AOS_KERNEL_DATA);
		print_hex(tb);
		prints("\n");
		if(tb==HANDLE_UNDEFINED)
		{
			prints("alloc min fault.\n");
		}
		else
		{
			boot_pmm_free(tb);
		}
		prints("alloc range:[0x");
		print_hex(0x40000000);
		prints(",0x");
		print_hex(0x14FFFFFFFF);
		prints("]=0x");
		tb=boot_pmm_alloc(MALLOC_RANGE,0x40000000,0x14FFFFFFFF,tp,AOS_KERNEL_DATA);
		print_hex(tb);
		prints("\n");
		if(tb==HANDLE_UNDEFINED)
		{
			prints("alloc range fault.\n");
		}
		else
		{
			boot_pmm_free(tb);
		}
		prints("alloc any:[0x");
		print_hex(0x40000000);
		prints(",0x");
		print_hex(0x14FFFFFFFF);
		prints("]=0x");
		tb=boot_pmm_alloc(MALLOC_ANY,0x40000000,0x14FFFFFFFF,tp,AOS_KERNEL_DATA);
		print_hex(tb);
		prints("\n");
		if(tb==HANDLE_UNDEFINED)
		{
			prints("alloc any fault.\n");
		}
		else
		{
			boot_pmm_free(tb);
		}
		prints("===\n");
		boot_pmm_debug_dump();
		prints("===\n");
		tp=(tp<<1)+1;
	}
}
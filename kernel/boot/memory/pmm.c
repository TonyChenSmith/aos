/*
 * 内存模块物理内存管理函数。
 * @date 2024-11-14
 */
#include "include/pmm.h"
#include "mem/mtype.h"
#include "type.h"

/*物理内存结点位映射*/
static uint64 node_bitmap[BOOT_PM_BITMAP]={0};

/*物理内存结点池基址*/
static boot_pm_node* pool=NULL;

/*结点池上限*/
static uintn pool_limit=0;

/*物理内存链表数组，0为头结点，1为尾结点，0-47为自由内存链表，48为已分配内存链表*/
static uintn pmem[2][49]={{[0 ... 48]=BOOT_PM_NODE},{[0 ... 48]=BOOT_PM_NODE}};

/*
 * 位映射初始化。
 *
 * @param param 启动参数结构指针。
 *
 * @return 无返回值。
 */
static void boot_pmm_bitmap_init(boot_params param)
{
	pool=(boot_pm_node*)((uintn)param.current_pointer&0xFFF?(((uintn)param.current_pointer>>12)+1)<<12:(uintn)param.current_pointer);
	uintn amount=0;
	if((uintn)pool+BOOT_PM_POOL>(uintn)param.pool+param.pool_length)
	{
		param.current_pointer=(void*)((uintn)param.pool+param.pool_length);
		pool_limit=((uintn)param.current_pointer-(uintn)pool)/sizeof(boot_pm_node);
		amount=BOOT_PM_NODE-pool_limit;
	}
	else
	{
		pool_limit=BOOT_PM_NODE;
		param.current_pointer=(void*)((uintn)pool+BOOT_PM_POOL);
	}
	uintn index=BOOT_PM_BITMAP-1;
	while(amount>=64)
	{
		amount=amount-64;
		node_bitmap[index--]=UINT64_MAX;
	}
	while(amount>0)
	{
		node_bitmap[index]=node_bitmap[index]>>1;
		node_bitmap[index]=node_bitmap[index]|0x8000000000000000ULL;
		amount--;
	}
}

/*
 * 给一个结点置零。
 * 
 * @param index 结点索引。
 *
 * @return 无返回值。
 */
static void boot_pmm_zero(const uintn index)
{
	__builtin_memset_inline(&pool[index],0,sizeof(boot_pm_node));
}

/*
 * 申请一个结点。
 *
 * @return 一个结点，失败返回结点索引边界。
 */
static uintn boot_pmm_nalloc(void)
{
	uintn bitmap=0;
	while(bitmap<BOOT_PM_BITMAP)
	{
		if(node_bitmap[bitmap]==UINT64_MAX)
		{
			bitmap++;
			continue;
		}
		else
		{
			uintn index=0;
			while(node_bitmap[bitmap]&((uintn)1<<index))
			{
				index++;
			}
			node_bitmap[bitmap]=node_bitmap[bitmap]|((uintn)1<<index);
			index=(bitmap<<6)+index;
			boot_pmm_zero(index);
			return index;
		}
	}
	return BOOT_PM_NODE;
}

/*
 * 释放一个结点。
 *
 * @param index 结点索引。
 *
 * @return 无返回值。
 */
static void boot_pmm_nfree(uintn index)
{
	if(index>=pool_limit)
	{
		return;
	}
	uintn bitmap=0;
	while(index>=64)
	{
		bitmap++;
		index=index-64;
	}
	node_bitmap[bitmap]=node_bitmap[bitmap]&(~((uintn)1<<index));
}

/*
 * 结点比较。
 *
 * @param node	 比较左式。
 * @param base	 比较右式基址。
 * @param amount 比较右式页数。
 *
 * @return 小于返回-1，包含或相等返回0，大于返回1，部分重叠返回2，覆盖返回3。
 */
static int boot_pmm_ncomp(const uintn node,const uintn base,const uintn amount)
{
	uintn base1=pool[node].base;
	uintn limit1=pool[node].base+(pool[node].amount<<12);
	uintn base2=base;
	uintn limit2=base+(amount<<12);
	if(base1<=base2&&limit1>=limit2)
	{
		return 0;
	}
	else if(limit1<=base2)
	{
		return -1;
	}
	else if(base1>=limit2)
	{
		return 1;
	}
	else if(base1>=base2&&limit1<=limit2)
	{
		return 3;
	}
	else
	{
		return 2;
	}
}

/*
 * 物理内存链表添加结点。
 *
 * @param list	 链表索引。
 * @param base	 页面基址。
 * @param amount 页面数目。
 * @param type	 内存类型。
 *
 * @return 成功返回真，失败返回假，包括页数为零，链表索引超出范围、没有内存或添加结点违反分离原则。
 */
static bool boot_pmm_nadd(const uintn list,const uintn base,const uintn amount,const mtype type)
{
	uintn node=boot_pmm_nalloc();
	if(list>=49||amount==0||node>=BOOT_PM_NODE)
	{
		return false;
	}
	pool[node].base=base;
	pool[node].amount=amount;
	pool[node].type=type;
	uintn head=pmem[0][list];
	if(head>=BOOT_PM_NODE)
	{
		pmem[0][list]=node;
		pmem[1][list]=node;
		pool[node].next=BOOT_PM_NODE;
		pool[node].prev=BOOT_PM_NODE;
		return true;
	}
	else
	{
		while(head<BOOT_PM_NODE)
		{
			switch(boot_pmm_ncomp(head,base,amount))
			{
				case -1:
					head=pool[head].next;
					continue;
				case 1:
					pool[node].next=head;
					uintn prev=pool[head].prev;
					pool[head].prev=node;
					if(prev>=BOOT_PM_NODE)
					{
						/*最小结点*/
						pool[node].prev=BOOT_PM_NODE;
						pmem[0][list]=node;
					}
					else
					{
						pool[prev].next=node;
						pool[node].prev=prev;
					}
					return true;
				case 0:
					/*独立区域，不应该重合*/
				case 2:
				case 3:
				default:
					boot_pmm_nfree(node);
					return false;
			}
		}
		/*最大结点*/
		head=pmem[1][list];
		pool[node].prev=head;
		pool[node].next=BOOT_PM_NODE;
		pool[head].next=node;
		pmem[1][list]=node;
		return true;
	}
}

/*
 * 物理内存链表删除结点。
 *
 * @param list 链表索引。
 * @param node 内存结点。
 *
 * @return 无返回值。
 */
static void boot_pmm_nremove(const uintn list,const uintn node)
{
	if(list>=49||node>=BOOT_PM_NODE)
	{
		return;
	}
	uintn head=pmem[0][list];
	while(head<BOOT_PM_NODE)
	{
		if(head==node)
		{
			uintn next=pool[head].next;
			head=pool[head].prev;
			if(head>=BOOT_PM_NODE)
			{
				pmem[0][list]=next;
				if(next>=BOOT_PM_NODE)
				{
					pmem[1][list]=BOOT_PM_NODE;
				}
				else
				{
					pool[next].prev=BOOT_PM_NODE;
				}
				break;
			}
			else if(next>=BOOT_PM_NODE)
			{
				pmem[1][list]=head;
				pool[head].next=BOOT_PM_NODE;
			}
			else
			{
				pool[next].prev=head;
				pool[head].next=next;
			}
			boot_pmm_nfree(node);
			return;
		}
		head=pool[head].next;
	}
}

/*
 * 物理内存拆分自由内存。
 *
 * @param base	 页面基址。
 * @param amount 页面数目。
 *
 * @return 成功返回真，失败返回假，主要是没有内存。
 */
static bool boot_pmm_fspilt(uintn base,uintn amount)
{
	while(amount>0)
	{
		/*0可以视为63位0，因为64位0的大块不能被64位寻址表示，由于结构问题，基址必有12个0*/
		uint8 mask=(uint8)__builtin_ctzg(base,(int)sizeof(uintn)*8-1);
		uint8 index=mask-12;
		uintn page=(uintn)1<<index;
		while(amount<page)
		{
			index--;
			page=(uintn)1<<index;
		}
		if(!boot_pmm_nadd(index,base,page,AVAILABLE))
		{
			return false;
		}
		else
		{
			base=base+(page<<12);
			amount=amount-page;
		}
	}
	return true;
}

/*
 * 物理内存合并自由内存。
 *
 * @return 成功返回真，失败返回假，主要是没有内存。
 */
static void boot_pmm_fmerga(void)
{
	for(uint8 index=0;index<47;index++)
	{
		uintn a=pmem[0][index];
		uintn length=(uintn)1<<(index+12);
		while(a<BOOT_PM_NODE&&pool[a].next<BOOT_PM_NODE)
		{
			/*index代表减12位后的位数，这里需要基址对应下一级的位数*/
			if(__builtin_ctzg(pool[a].base,(int)sizeof(uintn)*8-1)>=index+13&&pool[pool[a].next].base==pool[a].base+length)
			{
				uintn b=pool[a].next;
				uintn nbase=pool[a].base;
				uintn npage=length>>11;
				boot_pmm_nremove(index,a);
				boot_pmm_nremove(index,b);
				boot_pmm_nadd(index+1,nbase,npage,AVAILABLE);
				a=pmem[0][index];
			}
			else
			{
				a=pool[a].next;			
			}
		}
	}
}

/*
 * 将EFI内存类型转换为AOS内存类型。
 *
 * @param efi_type EFI内存类型。
 *
 * @return 返回对应AOS内存类型。
 */
static mtype boot_pmm_efi2mtype(const uint32 efi_type)
{
	switch(efi_type)
	{
		case EFI_RESERVED_MEMORY_TYPE:
		case EFI_UNACCEPTED_MEMORY_TYPE:
		case EFI_PERSISTENT_MEMORY:
		case EFI_PAL_CODE:
		case EFI_UNUSABLE_MEMORY:
		default:
			return RESERVED;
		case EFI_LOADER_CODE:
		case EFI_LOADER_DATA:
		case EFI_BOOT_SERVICES_CODE:
		case EFI_BOOT_SERVICES_DATA:
		case EFI_CONVENTIONAL_MEMORY:
			return AVAILABLE;
		case EFI_RUNTIME_SERVICES_CODE:
			return FW_CODE;
		case EFI_RUNTIME_SERVICES_DATA:
			return FW_DATA;
		case EFI_ACPI_RECLAIM_MEMORY:
			return ACPI_TABLE;
		case EFI_ACPI_MEMORY_NVS:
			return ACPI_NVS;
		case EFI_MEMORY_MAPPED_IO:
		case EFI_MEMORY_MAPPED_IO_PORT_SPACE:
			return MMIO;
	}
}

/*
 * 初始化物理内存管理列表。
 *
 * @param param 启动参数结构指针。
 *
 * @return 无返回值。
 */
extern void boot_pmm_init(const boot_params* restrict param)
{
	efi_memory_descriptor* dsc=param->env.memmap;
	uintn offset=param->env.entry_size;
	uintn end=param->env.memmap_length+(uintn)dsc;
	while((uintn)dsc<end)
	{
		mtype type=boot_pmm_efi2mtype(dsc->type);
		if(type!=AVAILABLE)
		{
			boot_pmm_nadd(48,dsc->physical_start,dsc->pages,type);
		}
		else
		{
			boot_pmm_fspilt(dsc->physical_start,dsc->pages);
		}
		dsc=(efi_memory_descriptor*)((uintn)dsc+offset);
	}
	boot_pmm_fmerga();
	void list(uintn list,uintn head,uintn tail);
	void line(uintn index,uintn node,uintn start,uintn end,uintn amount,uintn type);
	for(uintn index=0;index<49;index++)
	{
		list(index,pmem[0][index],pmem[1][index]);
	}
	for(uintn index=0;index<49;index++)
	{
		list(index,pmem[0][index],pmem[1][index]);
		uintn head=pmem[0][index];
		uintn i=0;
		while(head<BOOT_PM_NODE)
		{
			line(i++,head,pool[head].base,pool[head].base+(pool[head].amount<<12)-1,pool[head].amount,pool[head].type);
			head=pool[head].next;
		}
	}
}

/*
 * 申请一段指定物理内存。
 *
 * @param addr	地址参数。这里为指定位置的基址。
 * @param pages	页数。
 * @param type	申请内存类型。该模式无视内存类型优先级。
 *
 * @return 起始地址，或NULL。这里为了方便运算转为整数类型。
 */
static uintn boot_pmm_alloc_addr(const uintn addr,const uintn pages,const mtype type)
{
	return 0;
}

/*
 * 申请一段最大范围的物理内存。
 *
 * @param addr	地址参数。这里为开区间地址上限大小。
 * @param pages	页数。
 * @param type	申请内存类型。该模式无视内存类型优先级。
 *
 * @return 起始地址，或NULL。这里为了方便运算转为整数类型。
 */
static uintn boot_pmm_alloc_max(const uintn addr,const uintn pages,const mtype type)
{
	return 0;
}

/*
 * 申请一段物理内存。
 *
 * @param alloc_t 申请内存类型。
 * @param addr	  地址参数。仅在申请为最大地址或指定地址时需要。
 * @param pages	  页数。
 * @param type	  申请内存类型。
 *
 * @return 起始地址，或NULL。这里为了方便运算转为整数类型。
 */
extern uintn boot_pmm_alloc(const malloc_type alloc_t,const uintn addr,const uintn pages,const mtype type)
{
	return 0;
}
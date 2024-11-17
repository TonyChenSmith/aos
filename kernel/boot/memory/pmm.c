/*
 * 内存模块物理内存管理函数。
 * @date 2024-11-14
 */
#include "include/pmm.h"
#include "fw/efi.h"
#include "mem/mtype.h"
#include "param.h"
#include "type.h"

/*结点列表*/
static boot_pmm_node nodes[BOOT_PMML_SIZE]={0};

/*结点列表长度*/
static uintn nodes_size=0;

/*
 * 结点复制。
 *
 * @param dest 目标索引。
 * @param src  源索引。
 *
 * @return 无返回值。
 */
static void boot_pmm_nodecpy(const uintn dest,const uintn src)
{
	__builtin_memcpy_inline(&nodes[dest],&nodes[src],sizeof(boot_pmm_node));
}

/*
 * 结点比较。
 *
 * @param node	 结点。
 * @param base	 被比较结点基址。
 * @param amount 被比较结点页数。
 *
 * @return 小于返回-1，包含或相等返回0，大于返回1，部分重叠返回2，覆盖返回3。
 */
static int boot_pmm_nodecmp(const uintn node,const uintn base,const uintn amount)
{
	/*逻辑应该是index与输入比较*/
	uintn base1=nodes[node].base;
	uintn limit1=nodes[node].base+(nodes[node].amount<<12);
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
 * 添加物理内存结点。
 * 
 * @param base	 页面基址。
 * @param amount 页面数目。
 * @param type	 内存类型。
 *
 * @return 无返回值。
 */
static void boot_pmm_add(const uintn base,const uintn amount,const mtype type)
{
	nodes[nodes_size].base=base;
	nodes[nodes_size].amount=amount;
	nodes[nodes_size].type=type;
	nodes_size++;
}

/*
 * 插入物理内存结点。
 * 
 * @param index	 插入索引。
 * @param base	 页面基址。
 * @param amount 页面数目。
 * @param type	 内存类型。
 *
 * @return 成功返回真，没有空位或索引错误返回假。
 */
static bool boot_pmm_insert(const uintn index,const uintn base,const uintn amount,const mtype type)
{
	if(nodes_size>=BOOT_PMML_SIZE||index>nodes_size)
	{
		return false;
	}
	if(index==nodes_size)
	{
		boot_pmm_add(base,amount,type);
	}
	else
	{
		for(uintn i=nodes_size;i>index;i--)
		{
			boot_pmm_nodecpy(i,i-1);
		}
		nodes[index].base=base;
		nodes[index].amount=amount;
		nodes[index].type=type;
		nodes_size++;
	}
	return true;
}

/*
 * 分割物理内存结点。
 *
 * @param index	 结点索引。
 * @param base	 嵌入结点基址。
 * @param amount 嵌入结点页数。
 * @param type	 嵌入结点属性。
 *
 * @return 成功返回真，没有空位或索引错误返回假。
 */
static bool boot_pmm_split(const uintn index,const uintn base,const uintn amount,const mtype type)
{
	if(index>=nodes_size)
	{
		return false;
	}
	boot_pmm_node temp_nodes[3];
	bool node_exist[2];
	node_exist[0]=false;
	node_exist[1]=false;
	uint8 count=0;
	uintn base1=nodes[index].base;
	uintn limit1=nodes[index].base+(nodes[index].amount<<12);
	uintn base2=base;
	uintn limit2=base+(amount<<12);
	if(base1<base2)
	{
		/*存在前空缺*/
		node_exist[0]=true;
		count++;
		temp_nodes[0].base=base1;
		temp_nodes[0].amount=(base2-base1)>>12;
		temp_nodes[0].type=nodes[index].type;
	}
	temp_nodes[1].base=base2;
	temp_nodes[1].amount=amount;
	temp_nodes[1].type=type;
	if(limit2<limit1)
	{
		/*存在后空缺*/
		node_exist[1]=true;
		count++;
		temp_nodes[2].base=limit2;
		temp_nodes[2].amount=(limit1-limit2)>>12;
		temp_nodes[2].type=nodes[index].type;
	}
	if(nodes_size+count>=BOOT_PMML_SIZE)
	{
		return false;
	}
	else
	{
		for(uintn i=nodes_size+count;i>index+count;i--)
		{
			boot_pmm_nodecpy(i,i-1-count);
		}
		if(node_exist[0]&&node_exist[1])
		{
			__builtin_memcpy_inline(&nodes[index],&temp_nodes[0],3*sizeof(boot_pmm_node));
		}
		else if(node_exist[0])
		{
			__builtin_memcpy_inline(&nodes[index],&temp_nodes[0],2*sizeof(boot_pmm_node));
		}
		else if(node_exist[1])
		{
			__builtin_memcpy_inline(&nodes[index],&temp_nodes[1],2*sizeof(boot_pmm_node));
		}
		else
		{
			__builtin_memcpy_inline(&nodes[index],&temp_nodes[1],sizeof(boot_pmm_node));
		}
		nodes_size=nodes_size+count+1;
		return true;
	}
}

/*
 * 修改结点内存类型并合并相邻结点。
 *
 * @param index	结点索引。
 * @param type	结点类型。
 *
 * @return 成功返回真，索引错误返回假。
 */
static bool boot_pmm_merge(const uintn index,const mtype type)
{
	if(index>=nodes_size)
	{
		return false;
	}
	uintn base=nodes[index].base;
	uintn amount=nodes[index].amount;
	uintn i=index;
	uint8 count=0;
	if(index>0&&nodes[index-1].type==type&&base==(nodes[index-1].base+(nodes[index-1].amount<<12)))
	{
		i--;
		count++;
		base=nodes[index-1].base;
		amount=nodes[index-1].amount+amount;
	}
	if(index<nodes_size-1&&nodes[index+1].type==type&&nodes[index+1].base==(base+(amount<<12)))
	{
		count++;
		amount=amount+nodes[index+1].amount;
	}
	nodes[i].base=base;
	nodes[i].amount=amount;
	nodes[i].type=type;
	if(count>0)
	{
		nodes_size=nodes_size-count;
		for(i++;i<nodes_size;i++)
		{
			boot_pmm_nodecpy(i,i+count);
		}
	}
	return true;
}

/*
 * 查询列表区间。
 *
 * @param base	 结点基址。
 * @param amount 结点页数。
 *
 * @return 如果找到相等或重合的，返回对应索引。未命中返回-1，部分重叠返回-2，覆盖返回-3。
 */
static int boot_pmm_find(const uintn base,const uintn amount)
{
	if(nodes_size==0)
	{
		return -1;
	}
	/*二分查找*/
	uintn a=0,b=nodes_size-1;
	while(a<=b)
	{
		uintn mid=a+((b-a)>>1);
		switch(boot_pmm_nodecmp(mid,base,amount))
		{
			case -1:
				a=mid+1;
				break;
			case 1:
				b=mid;
				break;
			case 0:
				return (int)mid;
			case 2:
				return -2;
			case 3:
				return -3;
		}
	}
	return -1;
}

/*
 * 搜索最适合的插入点。这里假设没有重叠情形。
 *
 * @param base	 结点基址。
 * @param amount 结点页数。
 *
 * @return 返回最适合的插入点。
 */
static uintn boot_pmm_search(const uintn base,const uintn amount)
{
	for(int index=nodes_size-1;index>=0;index--)
	{
		if(boot_pmm_nodecmp(index,base,amount)==-1)
		{
			return index+1;
		}
	}
	return 0;
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
		uintn index=boot_pmm_search(dsc->physical_start,dsc->pages);
		mtype type=boot_pmm_efi2mtype(dsc->type);
		boot_pmm_insert(index,dsc->physical_start,dsc->pages,type);
		boot_pmm_merge(index,type);
		dsc=(efi_memory_descriptor*)((uintn)dsc+offset);
	}

	void line(uintn index,uintn start,uintn end,uintn amount,uintn type);
	for(uintn index=0;index<nodes_size;index++)
	{
		line(index,nodes[index].base,nodes[index].base+(nodes[index].amount<<12)-1,nodes[index].amount,nodes[index].type);
	}
}

extern void boot_pmm_alloc()
{
	_Alignof()
}
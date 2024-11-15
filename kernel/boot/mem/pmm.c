/*
 * 内存模块物理内存管理函数。
 * @date 2024-11-14
 */
#include "pmm.h"

/*结点列表*/
static boot_pmm_node nodes[BOOT_PMML_SIZE]={0};

/*结点列表长度*/
static uintn nodes_size=0;

/*
 * 添加物理内存结点。
 * 
 * @param base	 页面基址。
 * @param amount 页面数目。
 * @param type	 内存类型。
 *
 * @return 成功返回真，没有空位返回假。
 */
static bool boot_pmm_add(const uintn base,const uintn amount,const aos_memory_type type)
{
	if(nodes_size>=BOOT_PMML_SIZE)
	{
		return false;
	}
	nodes[nodes_size].base=base;
	nodes[nodes_size].amount=amount;
	nodes[nodes_size].type=type;
	nodes_size++;
	return true;
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
static bool boot_pmm_insert(const uintn index,const uintn base,const uintn amount,const aos_memory_type type)
{
	if(nodes_size>=BOOT_PMML_SIZE||index>nodes_size)
	{
		return false;
	}
	if(index==nodes_size)
	{
		return boot_pmm_add(base,amount,type);
	}
	else
	{
		for(uintn i=index;i<nodes_size;i++)
		{
			__builtin_memcpy_inline(&nodes[i+1],&nodes[i],sizeof(boot_pmm_node));
		}
		nodes[index].base=base;
		nodes[index].amount=amount;
		nodes[index].type=type;
	}
	nodes_size++;
	return true;
}
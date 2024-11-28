/*
 * 物理内存管理函数。
 * @date 2024-11-14
 */
#ifndef __AOS_BOOT_MEMORY_PMM_H__
#define __AOS_BOOT_MEMORY_PMM_H__

#include "type.h"
#include "config.h"
#include "mem/mtype.h"
#include "param.h"

/*物理内存结点*/
typedef struct _boot_pm_node
{
	uintn base;	  /*页面基址*/
	uintn amount; /*页面数目*/
	mtype type;	  /*内存类型*/
	uint32 prev;  /*前一结点*/
	uint32 next;  /*下一结点*/
} boot_pm_node;

/*物理内存结点最大数目*/
#define BOOT_PM_NODE (BOOT_PM_POOL/sizeof(boot_pm_node))

/*物理内存结点位映射*/
#define BOOT_PM_BITMAP (BOOT_PM_NODE/64)

/*
 * 申请一段物理内存。
 *
 * @param max	是否有最大上限。
 * @param addr	地址参数。仅在申请为最大地址时需要。
 * @param pages	页数。
 * @param type	申请内存类型。
 *
 * @return 起始地址，或NULL。这里为了方便运算转为整数类型。
 */
extern uintn boot_pmm_alloc(const bool max,const uintn addr,const uintn pages,const mtype type);

/*
 * 释放已分配自由内存。
 *
 * @param pointer 地址。并不强制要求在申请区间的基地址，在申请区间有效范围就行。
 *
 * @return 无返回值。
 */
extern void boot_pmm_free(const uintn pointer);

/*
 * 映射一段内存，不需要验证其合法性，该函数仅记录映射。
 *
 * @param base	内存映射基址。
 * @param pages 页面数目。
 * @param type	内存类型。
 *
 * @return 无返回值。
 */
extern void boot_pmm_map(const uintn base,const uintn pages,const mtype type);

/*物理内存句柄*/
typedef khandle phandle;

/*
 * 获取映射列表的迭代器。该迭代器仅有查看的功能。
 *
 * @return 新迭代器。
 */
extern phandle boot_pmm_map_iterator(void);

/*
 * 获取已分配列表的迭代器。该迭代器仅有查看的功能。
 *
 * @return 新迭代器。
 */
extern phandle boot_pmm_allocated_iterator(void);

/*
 * 获取迭代器当前结点的结点信息。
 *
 * @param handle 迭代器句柄。
 * @param base	 结点基址。
 * @param amount 结点页数。
 * @param type	 结点内存类型。
 *
 * @return 成功返回真，到达结尾或结点不存在返回假。
 */
extern bool boot_pmm_nread(phandle handle,uintn* restrict base,uintn* restrict amount,mtype* restrict type);

/*
 * 获取迭代器的下一个结点。
 * 
 * @return 下一个结点。
 */
extern phandle boot_pmm_nnext(phandle handle);

#endif /*__AOS_BOOT_MEM_PMM_H__*/
/*
 * 虚拟内存管理函数。
 * @date 2024-11-30
 */
#ifndef __AOS_BOOT_MEMORY_VMM_H__
#define __AOS_BOOT_MEMORY_VMM_H__

#include "type.h"
#include "config.h"
#include "mem/mtype.h"
#include "module/base.h"
#include "param.h"
#include "pmm.h"

/*内存映射结点*/
typedef struct _boot_mmap_node
{
	uintn pbase;				 /*物理基址*/
	uintn vbase;				 /*虚拟基址*/
	uintn pages;				 /*占用页数*/
	uint16 type;				 /*内存类型*/
	uint16 attr;				 /*内存属性*/
	uint32 pid;					 /*处理器编号*/
	struct _boot_vpm_node* next; /*下一个结点*/
} boot_mmap_node;

/*内存映射分配池结点*/
typedef struct _boot_mmp_node
{
	struct _boot_mmp_node* prev; /*前一个结点*/
	struct _boot_mmp_node* next; /*后一个结点*/
	uint16 nodes;				 /*管理结点总数*/
	uint16 free;				 /*空闲结点*/
	uint16 bitmap;				 /*位映射数组长度，单位为一个64位无符号整数*/
	uint16 offset;				 /*结点管辖页面偏移，一般为一个页面大小*/
} boot_mmp_node;

/*4GB边界*/
#define LIMIT_4GB 0x100000000ULL

/*配置范围检查*/
#if BOOT_MM_POOL<400||BOOT_MM_POOL>((4096-24)<<3)
#error The value of BOOT_MM_POOL is not within the supported range.
#endif

/*单个内存映射分配池所需页数*/
#define BOOT_MMP_PAGE (((((uintn)BOOT_MM_POOL)*sizeof(boot_mmap_node))>>12)+((((uintn)BOOT_MM_POOL)*sizeof(boot_mmap_node))&0xFFF?1:0)+1)

#endif /*__AOS_BOOT_MEMORY_VMM_H__*/
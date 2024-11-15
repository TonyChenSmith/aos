/*
 * 内存模块物理内存管理函数声明。
 * @date 2024-11-14
 */
#ifndef __AOS_BOOT_MEM_PMM_H__
#define __AOS_BOOT_MEM_PMM_H__

#include "type.h"
#include "boot.h"
#include "memtype.h"

/*物理内存结点*/
typedef struct _boot_pmm_node
{
	uintn base;			  /*页面基址*/
	uintn amount;		  /*页面数目*/
	aos_memory_type type; /*内存类型*/
} boot_pmm_node;

#endif /*__AOS_BOOT_MEM_PMM_H__*/
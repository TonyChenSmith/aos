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
typedef struct _boot_pmm_node
{
	uintn base;	  /*页面基址*/
	uintn amount; /*页面数目*/
	mtype type;	  /*内存类型*/
} boot_pmm_node;

#endif /*__AOS_BOOT_MEM_PMM_H__*/
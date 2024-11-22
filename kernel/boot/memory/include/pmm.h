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

#endif /*__AOS_BOOT_MEM_PMM_H__*/
/*
 * 页框定义。
 * @date 2024-12-12
 */
#ifndef __AOS_MEMORY_PAGE_FRAME_H__
#define __AOS_MEMORY_PAGE_FRAME_H__

#include "basic_type.h"

/*物理页框*/
typedef struct _physical_page_frame
{
	handle prev; /*前一页框句柄*/
	handle next; /*后一页框句柄*/
	uintn base;	 /*物理基址*/
	uintn pages; /*页框页数*/
	uint16 type; /*内存类型*/
} physical_page_frame;

/*虚拟页框*/
typedef struct _virtual_page_frame
{
	handle prev; /*前一页框句柄*/
	handle next; /*后一页框句柄*/
	uintn pbase; /*物理基址*/
	uintn vbase; /*虚拟基址*/
	uintn pages; /*页框页数*/
	uint16 attr; /*内存属性*/
	uint16 type; /*内存类型*/
	uint32 pid;	 /*所属处理器编号*/
} virtual_page_frame;

#endif
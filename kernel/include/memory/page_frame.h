/*
 * 页框定义。
 * @date 2024-12-12
 */
#ifndef __AOS_MEMORY_PAGE_FRAME_H__
#define __AOS_MEMORY_PAGE_FRAME_H__

#include "util/tree.h"

/*物理页框*/
typedef struct _physical_page_frame
{
	tree_node node;	/*树结点*/
	/*unused2*/		/*内存类型*/
	/*unused3*/		/*所属处理器编号*/
	uintn base;		/*物理基址*/
	uintn pages;	/*页框页数*/
} physical_page_frame;

/*虚拟页框*/
typedef struct _virtual_page_frame
{
	tree_node node;	/*树结点*/
	/*unused2*/		/*内存类型*/
	/*unused3*/		/*内存属性*/
	uintn pbase;	/*物理基址*/
	uintn vbase;	/*虚拟基址*/
	uintn pages;	/*页框页数*/
} virtual_page_frame;

/*获取页框内存类型*/
#define page_frame_get_type(page_frame) ((page_frame)->node.unused2)

/*设置页框内存类型*/
#define page_frame_set_type(page_frame,type) (page_frame)->node.unused2=(uint16)(type)

/*获取页框所属处理器编号*/
#define page_frame_get_pid(page_frame) ((page_frame)->node.unused3)

/*获取页框内存属性*/
#define page_frame_get_attr(page_frame) ((page_frame)->node.unused3)

/*设置页框所属处理器编号*/
#define page_frame_set_pid(page_frame,pid) (page_frame)->node.unused3=(uint32)(pid)

/*设置页框内存属性*/
#define page_frame_set_attr(page_frame,attr) (page_frame)->node.unused3=(uint32)(attr)

/*内存属性定义*/

/*可读*/
#define PAGE_FRAME_READ 0x00000004

/*可写*/
#define PAGE_FRAME_WRITE 0x00000002

/*可执行*/
#define PAGE_FRAME_EXECUTABLE 0x00000001

/*内核级别*/
#define PAGE_FRAME_KERNEL 0x00000000

/*用户级别*/
#define PAGE_FRAME_USER 0x0000000F

/*无缓存*/
#define PAGE_FRAME_UC 0x00000010

/*写合并*/
#define PAGE_FRAME_WC 0x00000020

/*写通*/
#define PAGE_FRAME_WT 0x00000030

/*写保护*/
#define PAGE_FRAME_WP 0x00000040

/*写回*/
#define PAGE_FRAME_WB 0x00000050

/*全局页*/
#define PAGE_FRAME_GLOBAL 0x000000F0

#endif
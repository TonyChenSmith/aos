/*
 * 页表管理函数。
 * @date 2024-11-15
 */
#ifndef __AOS_BOOT_MEMORY_PTM_H__
#define __AOS_BOOT_MEMORY_PTM_H__

#include "type.h"
#include "config.h"
#include "module/base.h"
#include "param.h"
#include "builtin.h"
#include "pmm.h"

#pragma pack(1)

/*页表项*/
typedef union _page_entry
{
	uint64 value;			 /*页表项值*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 :6;
		uint32 ps:1;		 /*页尺寸*/
		uint32 :24;
		uint32 :32;
	} pt_entry;				 /*BPTE*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 rw:1;		 /*读写*/
		uint32 us:1;		 /*权限*/
		uint32 pwt:1;		 /*页直写*/
		uint32 pcd:1;		 /*页缓存*/
		uint32 a:1;			 /*访问*/
		uint32 :1;
		uint32 ps:1;		 /*页尺寸*/
		uint32 :4;
		uint32 addr_down:20; /*物理地址下半*/
		uint32 addr_up:20;	 /*物理地址上半*/
		uint32 :11;
		uint32 xd:1;		 /*NXE*/
	} pml_entry;			 /*PML4E、PML5E*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 rw:1;		 /*读写*/
		uint32 us:1;		 /*权限*/
		uint32 pwt:1;		 /*页直写*/
		uint32 pcd:1;		 /*页缓存*/
		uint32 a:1;			 /*访问*/
		uint32 d:1;			 /*脏页*/
		uint32 ps:1;		 /*页尺寸*/
		uint32 g:1;			 /*全局*/
		uint32 :3;
		uint32 pat:1;		 /*PAT*/
		uint32 addr_down:19; /*物理地址下半*/
		uint32 addr_up:20;	 /*物理地址上半*/
		uint32 :7;
		uint32 pk:4;		 /*PK*/
		uint32 xd:1;		 /*NXE*/
	} page_1gb_entry;		 /*PE1GB*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 rw:1;		 /*读写*/
		uint32 us:1;		 /*权限*/
		uint32 pwt:1;		 /*页直写*/
		uint32 pcd:1;		 /*页缓存*/
		uint32 a:1;			 /*访问*/
		uint32 :1;
		uint32 ps:1;		 /*页尺寸*/
		uint32 :4;
		uint32 addr_down:20; /*物理地址下半*/
		uint32 addr_up:20;	 /*物理地址上半*/
		uint32 :11;
		uint32 xd:1;		 /*NXE*/
	} pdpt_entry;			 /*PDPTE*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 rw:1;		 /*读写*/
		uint32 us:1;		 /*权限*/
		uint32 pwt:1;		 /*页直写*/
		uint32 pcd:1;		 /*页缓存*/
		uint32 a:1;			 /*访问*/
		uint32 d:1;			 /*脏页*/
		uint32 ps:1;		 /*页尺寸*/
		uint32 g:1;			 /*全局*/
		uint32 :3;
		uint32 pat:1;		 /*PAT*/
		uint32 addr_down:19; /*物理地址下半*/
		uint32 addr_up:20;	 /*物理地址上半*/
		uint32 :7;
		uint32 pk:4;		 /*PK*/
		uint32 xd:1;		 /*NXE*/
	} page_2mb_entry;		 /*PG2MB*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 rw:1;		 /*读写*/
		uint32 us:1;		 /*权限*/
		uint32 pwt:1;		 /*页直写*/
		uint32 pcd:1;		 /*页缓存*/
		uint32 a:1;			 /*访问*/
		uint32 :1;
		uint32 ps:1;		 /*页尺寸*/
		uint32 :4;
		uint32 addr_down:20; /*物理地址下半*/
		uint32 addr_up:20;	 /*物理地址上半*/
		uint32 :11;
		uint32 xd:1;		 /*NXE*/
	} pd_entry;				 /*PDE*/
	struct
	{
		uint32 p:1;			 /*存在*/
		uint32 rw:1;		 /*读写*/
		uint32 us:1;		 /*权限*/
		uint32 pwt:1;		 /*页直写*/
		uint32 pcd:1;		 /*页缓存*/
		uint32 a:1;			 /*访问*/
		uint32 d:1;			 /*脏页*/
		uint32 pat:1;		 /*PAT*/
		uint32 g:1;			 /*全局*/
		uint32 :3;
		uint32 addr_down:20; /*物理地址下半*/
		uint32 addr_up:20;	 /*物理地址上半*/
		uint32 :7;
		uint32 pk:4;		 /*PK*/
		uint32 xd:1;		 /*NXE*/
	} page_4kb_entry;		 /*PTE*/
} page_entry;

#pragma pack()

/*页表分配池结点*/
typedef struct _boot_ptp_node
{
	struct _boot_ptp_node* prev; /*前一个结点*/
	struct _boot_ptp_node* next; /*后一个结点*/
	uint16 pages;				 /*管理页面总数*/
	uint16 free;				 /*空闲页面数目*/
	uint16 bitmap;				 /*位映射数组长度，单位为一个64位无符号整数*/
	uint16 offset;				 /*结点管辖页面偏移，一般为一个页面大小*/
} boot_ptp_node;

/*配置范围检查*/
#if BOOT_PT_POOL<10||BOOT_PT_POOL>((4096-24)<<3)
#error The value of BOOT_PT_POOL is not within the supported range.
#endif

/*单个页面分配池位映射数组长度*/
#define BOOT_PT_BITMAP (((uintn)BOOT_PT_POOL)>>6)

/*单个页面分配池页数*/
#define BOOT_PT_PAGE (BOOT_PT_BITMAP>>6)

#endif /*__AOS_BOOT_MEM_PTM_H__*/
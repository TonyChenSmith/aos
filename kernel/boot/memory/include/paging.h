/*
 * 分页管理函数。
 * @date 2024-11-15
 */
#ifndef __AOS_BOOT_MEMORY_PAGING_H__
#define __AOS_BOOT_MEMORY_PAGING_H__

#include "type.h"
#include "config.h"
#include "module/base.h"
#include "param.h"
#include "builtin.h"

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

/*页面分配池页数*/
#define BOOT_PTP_PAGE (BOOT_PTP_SIZE>>12)

/*页面分配池位映射数组长度*/
#define BOOT_PTP_BITMAP (BOOT_PTP_PAGE>>6)

#endif /*__AOS_BOOT_MEM_PAGING_H__*/
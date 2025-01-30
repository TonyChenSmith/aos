/*
 * aos.boot.mem模块公开函数接口。
 * @date 2024-10-28
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_MEMORY_H__
#define __AOS_BOOT_MEMORY_H__

#include "params.h"
#include "func_table.h"
#include "memory/memory_defs.h"

#pragma pack(1)

/*IA32页表项*/
typedef union _ia32_page_entry
{
	uint64 value;
	struct
	{
		uint32 p:1;
		uint32 :6;
		uint32 ps:1;
		uint32 :24;
		uint32 :32;
	} pt_entry; /*BPTE*/
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 :1;
		uint32 ps:1;
		uint32 :4;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :11;
		uint32 xd:1;
	} pml_entry; /*PML4E、PML5E*/
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 d:1;
		uint32 ps:1;
		uint32 g:1;
		uint32 :3;
		uint32 pat:1;
		uint32 addr_down:19;
		uint32 addr_up:20;
		uint32 :7;
		uint32 pk:4;
		uint32 xd:1;
	} page_1gb_entry; /*PE1GB*/
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 :1;
		uint32 ps:1;
		uint32 :4;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :11;
		uint32 xd:1;
	} pdpt_entry; /*PDPTE*/
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 d:1;
		uint32 ps:1;
		uint32 g:1;
		uint32 :3;
		uint32 pat:1;
		uint32 addr_down:19;
		uint32 addr_up:20;
		uint32 :7;
		uint32 pk:4;
		uint32 xd:1;
	} page_2mb_entry; /*PG2MB*/
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 :1;
		uint32 ps:1;
		uint32 :4;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :11;
		uint32 xd:1;
	} pd_entry; /*PDE*/
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 d:1;
		uint32 pat:1;
		uint32 g:1;
		uint32 :3;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :7;
		uint32 pk:4;
		uint32 xd:1;
	} page_4kb_entry; /*PTE*/
} ia32_page_entry;

#pragma pack()

/*页面地址掩码*/

/*2MB大页地址掩码*/
#define PAGE_2MB_MASK 0x1FFFFF

/*1GB大页地址掩码*/
#define PAGE_1GB_MASK 0x3FFFFFFF

/*页面属性*/

/*页面只读*/
#define PAGE_ATTRIBUTE_R 0x00000000

/*页面可写*/
#define PAGE_ATTRIBUTE_W 0x00000001

/*页面管理员权限*/
#define PAGE_ATTRIBUTE_S 0x00000000

/*页面用户权限*/
#define PAGE_ATTRIBUTE_U 0x00000002

/*页面全局*/
#define PAGE_ATTRIBUTE_G 0x00000004

/*页面不可运行*/
#define PAGE_ATTRIBUTE_NX 0x00000008

/*页面不可缓存*/
#define PAGE_ATTRIBUTE_UC 0x00000000

/*页面写集中*/
#define PAGE_ATTRIBUTE_WC 0x00000010

/*页面直写*/
#define PAGE_ATTRIBUTE_WT 0x00000020

/*页面回写*/
#define PAGE_ATTRIBUTE_WB 0x00000040

/*页面写保护*/
#define PAGE_ATTRIBUTE_WP 0x00000080

/*页面类型掩码*/
#define PAGE_ATTRIBUTE_TYPE_MASK 0x000000F0

/*页面辅助宏*/

/*地址对齐2MB*/
#define boot_align_2mb(addr) (addr&PAGE_2MB_MASK?false:true)

/*地址对齐1GB*/
#define boot_align_1gb(addr) (addr&PAGE_1GB_MASK?false:true)

/*获取读写属性*/
#define boot_attr_rw(attr) (attr&PAGE_ATTRIBUTE_W)

/*获取权限属性*/
#define boot_attr_us(attr) ((attr&PAGE_ATTRIBUTE_U)>>1)

/*获取全局属性*/
#define boot_attr_g(attr) ((attr&PAGE_ATTRIBUTE_G)>>2)

/*获取页面类型*/
#define boot_attr_type(attr) ((attr&PAGE_ATTRIBUTE_TYPE_MASK)>>4)

/*
 * 物理内存管理系统(PMMS)初始化。
 *
 * @param global 全局函数表。
 * @param params 启动核参数。
 * 
 * @return 无返回值。
 */
typedef void (*boot_memory_pmm_init)(const boot_function_table* global,const boot_params* params);

/*
 * 物理内存申请，其范围为闭区间。
 * 
 * @param mode	申请模式。
 * @param min	申请范围下限。
 * @param max	申请范围上限。
 * @param pages 申请页数。
 * @param type	申请类型。
 *
 * @return 对应内存基址，失败返回未定义。 
 */
typedef uintn (*boot_memory_pmm_alloc)(const malloc_mode mode,const uintn min,const uintn max,const uintn pages,const memory_type type);

/*
 * 物理内存释放。
 * 
 * @param addr 需要释放区域内的地址。
 *
 * @return 无返回值。 
 */
typedef void (*boot_memory_pmm_free)(const uintn addr);

/*
 * 内存模块函数表。
 */
typedef struct _boot_memory_function
{
	boot_memory_pmm_init pmm_init;	 /*PMMS初始化*/
	boot_memory_pmm_alloc pmm_alloc; /*物理内存申请*/
	boot_memory_pmm_free pmm_free;	 /*物理内存释放*/
} boot_memory_function;

#endif /*__AOS_BOOT_MEMORY_H__*/
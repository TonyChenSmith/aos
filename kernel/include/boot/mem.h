/*
 * 内存模块函数和结构定义。定义了所有导出函数和入口函数。
 * @date 2024-10-28
 */
#ifndef __AOS_BOOT_MEM_H__
#define __AOS_BOOT_MEM_H__

#include "type.h"
#include "param.h"
#include "base.h"

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
	} page_entry; //基础表项
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 :6;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :11;
		uint32 xd:1;
	} pml_entry; //目录级别 pml5 pml4
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
	} page_1gb_entry; //1gb
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 :6;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :11;
		uint32 xd:1;
	} pdpt_entry;
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
	} page_2mb_entry; //ps页表级别
	struct
	{
		uint32 p:1;
		uint32 rw:1;
		uint32 us:1;
		uint32 pwt:1;
		uint32 pcd:1;
		uint32 a:1;
		uint32 :6;
		uint32 addr_down:20;
		uint32 addr_up:20;
		uint32 :11;
		uint32 xd:1;
	} pd_entry;
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
	} page_4kb_entry; //页表级别 4k页表
} ia32_page_entry;

#pragma pack()

/*
 * 初始化内存空间。其内有四个步骤：设置CPU寄存器、构造页表、切换页表和切换运行栈。
 *
 * @param params	 启动参数结构指针。
 * @param base_funcs 基础模块函数表指针。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory(const boot_params* params,const boot_base_functions* base_funcs);

#endif /*__AOS_BOOT_MEM_H__*/
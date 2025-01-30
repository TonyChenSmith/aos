/*
 * 内存模块初始化函数。
 * @date 2024-10-28
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "fw/efi.h"
#include "include/boot_bitmap.h"
#include "memory/memory_defs.h"
#include "memory/page_frame.h"
#include "module/base.h"
#include "basic_type.h"
#include "param.h"
#include "include/pmm.h"
#include "util/bitmap_pool.h"
#include "include/boot_tree.h"

static uintn stack_base=HANDLE_UNDEFINED;
static uintn stack_length=HANDLE_UNDEFINED;
extern void boot_init_memory_step2(const boot_params* param,const boot_base_functions* bbft,const uintn stack);

/*
 * 初始化内存空间第一阶段。负责初始化物理伙伴系统，并在物理伙伴系统中申请栈并切换栈。
 *
 * @param param	启动参数结构。
 * @param bbft	基础模块函数表。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory_step1(boot_params* restrict param,const boot_base_functions* restrict bbft)
{
	boot_pmm_init(param,bbft);
	stack_length=(BOOT_INITIAL_STACK*0x200)<<12;
	stack_base=boot_pmm_alloc(MALLOC_MAX,0,UINT32_MAX,BOOT_INITIAL_STACK*0x200,AOS_KERNEL_CODE);
	if(stack_base==HANDLE_UNDEFINED)
	{
		/*可用内存不足，可能是参数设置问题，也有可能是预设内存池大小问题*/
		DEBUG_START
		/*错误：在低4GB内存空间内，系统没有足够内存申请初始内核栈，请尝试重新配置BOOT_INITIAL_STACK。系统因此将在此处关机。*/
		#define BOOT_INITS_ERROR_MSG "Error:The system does not have enough memory to apply for the initial kernel stack in the low 4GB memory space,please try to reconfigure the BOOT_INITIAL_STACK.The system will therefore shut down here.\n"
		bbft->boot_writeport(BOOT_INITS_ERROR_MSG,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(BOOT_INITS_ERROR_MSG)-1);
		DEBUG_END
		bbft->boot_ms_call_4(param->runtime->reset_system,EFI_RESET_SHUTDOWN,EFI_OUT_OF_RESOURCES,0,0);
		__builtin_unreachable();
	}
	boot_init_memory_step2(param,bbft,stack_base+stack_length);
	__builtin_unreachable();
}

/*
 * 初始化内存空间。其内有四个步骤：构造页表、切换页表、设置CPU寄存器和切换运行栈。
 *
 * @param params 启动参数结构。
 * @param bfuncs 基础模块函数表。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory_step3(boot_params* params,const boot_base_functions* bfuncs)
{
	prints("hello,aos\n");
	while(1)
	{
		__asm__("hlt"::);
	}
	__builtin_unreachable();
}

static 

/*
 * 获取内存模块函数表。
 * 
 * @return 函数表指针。
 */
extern void* boot_memory_table(void)
{

}
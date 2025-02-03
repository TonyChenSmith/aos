/*
 * 工具模块函数表获取。
 * @date 2025-01-17
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/init.h"

/*函数表*/
static boot_util_function table;

/*
 * 模块初始化，设置模块所需功能的前置配置。
 *
 * @return 无返回值。
 */
static void boot_module_init(void)
{
	/*为Intel CPU开启ERMS*/
	uint32 eax=0,ebx=0,ecx=0,edx=0;
	if(__get_cpuid_count(0x7,0,&eax,&ebx,&ecx,&edx))
	{
		if(ebx&0x200)
		{
			/*EBX第9位为ERMS功能支持位，写入IA32_MISC_ENABLE(0x1A0)[bit0]开启*/
			builtin_wrmsr(0x1A0,builtin_rdmsr(0x1A0)|1);
		}
	}
}

/*
 * 获取工具模块函数表。
 * 
 * @return 函数表指针。
 */
extern void* boot_util_table(void)
{
	table.init=boot_module_init;
	table.ms_call_2=boot_ms_call_2;
	table.ms_call_4=boot_ms_call_4;
	table.memcpy=boot_memcpy;
	table.memset=boot_memset;
	table.readport=boot_readport;
	table.writeport=boot_writeport;
	return &table;
}
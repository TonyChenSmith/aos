/*
 * 初始化流程实现，从切换ABI到完全进入0号进程。
 * @date 2025-01-17
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/init.h"

/*全局函数表初始化*/
boot_function_table global={0};

/*
 * 初始化第一阶段。构造函数表和PMMS。
 *
 * @param params 启动核参数。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_step1(boot_params* restrict params)
{
	global.util=boot_init_table(params->modules[BOOT_UTIL_MODULE]);
	global.memory=boot_init_table(params->modules[BOOT_MEMORY_MODULE]);

	#define MSG1 "Hello AOS!\n"
	global.util->writeport(MSG1,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(MSG1)-1);
	
	while(1)
	{
		__asm__("hlt"::);
	}
	__builtin_unreachable();
}
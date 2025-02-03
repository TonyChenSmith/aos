/*
 * 函数表获取与内存模块初始化实现。
 * @date 2025-02-02
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/init.h"

/*函数表*/
static boot_memory_function table;

/*
 * 获取内存模块函数表。
 * 
 * @return 函数表指针。
 */
extern void* boot_memory_table(void)
{
	table.pmm_init=boot_pmm_init;
	table.pmm_alloc=boot_pmm_alloc;
	table.pmm_free=boot_pmm_free;
	return &table;
}
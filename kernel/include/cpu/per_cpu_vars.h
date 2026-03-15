/**
 * 内核每CPU变量管理。
 * @date 2026-03-11
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_CPU_PER_CPU_VARS_H__
#define __AOS_KERNEL_CPU_PER_CPU_VARS_H__

#include <support/type.h>

/**
 * 每CPU变量。
 */
typedef enum _per_cpu_variable
{
    PER_CPU_PAGE /*每CPU页池*/
} per_cpu_variable;

/**
 * 获取每CPU变量值。
 * 
 * @param variable 每CPU变量。
 * 
 * @return 变量对应值，无设置或无对应返回0。
 */
uint64 get_per_cpu_variable(per_cpu_variable variable);

/**
 * 设置每CPU变量值。如果没有初始化该CPU的每CPU变量区域则初始化。
 * 
 * @param variable 每CPU变量。
 * @param value    变量值。
 * 
 * @return 上一次存储的变量值。
 */
uint64 set_per_cpu_variable(per_cpu_variable variable,uint64 value);

#endif /*__AOS_KERNEL_CPU_PER_CPU_VARS_H__*/
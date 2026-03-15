/**
 * 内核CPU信息获取。
 * @date 2026-03-10
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_CPU_INFO_H__
#define __AOS_KERNEL_CPU_INFO_H__

#include <support/type.h>

/**
 * 获取当前运行CPU的编号。
 * 
 * @return 当前CPU编号。
 */
uint32 get_current_cpu_id();

/**
 * 获取当前运行CPU的是否是引导处理器。
 * 
 * @return 如果是BSP返回真。
 */
bool is_bootstrap_processor();

#endif /*__AOS_KERNEL_CPU_INFO_H__*/
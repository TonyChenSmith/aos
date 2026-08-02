/**
 * 内核CPU管理模块内部声明和定义。
 * @date 2026-06-14
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_CPU_CPU_INTERNAL_H__
#define __AOS_KERNEL_CPU_CPU_INTERNAL_H__

#include <support/type.h>

/**
 * 使用FXSAVE64/FXRSTOR64管理FPU状态。
 */
#define FPU_CONTROL_FXSR 0

/**
 * 使用XSAVE/XRSTOR管理扩展状态。
 */
#define FPU_CONTROL_XSR 1

/**
 * 使用XSAVEOPT/XRSTOR管理扩展状态。
 */
#define FPU_CONTROL_XOSR 2

/**
 * 设置FPU控制模式。
 * 
 * @param mode  控制模式编号。
 * @param align 保存区域对齐位数。
 * @param size  保存区域大小。
 * 
 * @return 无返回值。
 */
void cpu_set_fpu_control_mode(uintn mode,uintn align,uintn size);

#endif /*__AOS_KERNEL_CPU_CPU_INTERNAL_H__*/
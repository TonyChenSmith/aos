/**
 * 内核CPU中断控制。
 * @date 2026-04-23
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_CPU_INTERRUPT_H__
#define __AOS_KERNEL_CPU_INTERRUPT_H__

#include <support/type.h>

/**
 * 获取中断启用状态。
 * 
 * @return 中断开启状态，开启为真。
 */
bool get_interrupt_status(void);

/**
 * 保存并禁用中断。
 * 
 * @return 禁用前中断开启状态，开启为真。
 */
bool save_and_disable_interrupt(void);

/**
 * 恢复中断状态。
 * 
 * @param status 中断状态。真为启用。
 * 
 * @return 无返回值。
 */
void restore_interrupt(bool status);

/**
 * 强制启用中断。
 * 
 * @return 无返回值。
 */
void enable_interrupt(void);

/**
 * 强制禁用中断。
 * 
 * @return 无返回值。
 */
void disable_interrupt(void);

#endif /*__AOS_KERNEL_CPU_INTERRUPT_H__*/
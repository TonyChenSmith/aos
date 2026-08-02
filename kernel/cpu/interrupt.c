/**
 * 内核CPU中断控制。
 * @date 2026-06-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <cpu/interrupt.h>
#include <support/control.h>
#include <support/io.h>

/**
 * 获取中断启用状态。
 * 
 * @return 中断开启状态，开启为真。
 */
bool get_interrupt_status(void)
{
    return x86_read_flags()&BIT9;
}

/**
 * 保存并禁用中断。
 * 
 * @return 禁用前中断开启状态，开启为真。
 */
bool save_and_disable_interrupt(void)
{
    bool status=x86_read_flags()&BIT9;
    x86_disable_interrupt();
    return status;
}

/**
 * 恢复中断状态。
 * 
 * @param status 中断状态。真为启用。
 * 
 * @return 无返回值。
 */
void restore_interrupt(bool status)
{
    if(status)
    {
        x86_enable_interrupt();
    }
    else
    {
        x86_disable_interrupt();
    }
}

/**
 * 强制启用中断。
 * 
 * @return 无返回值。
 */
void enable_interrupt(void)
{
    x86_enable_interrupt();
}

/**
 * 强制禁用中断。
 * 
 * @return 无返回值。
 */
void disable_interrupt(void)
{
    x86_disable_interrupt();
}
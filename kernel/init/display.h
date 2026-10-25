/**
 * 内核临时显示。
 * @date 2026-10-18
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_INIT_DISPLAY_H__
#define __AOS_KERNEL_INIT_DISPLAY_H__

#include <init/params.h>

/**
 * 初始化屏幕。
 * 
 * @param params 启动参数。
 * 
 * @return 成功返回真。
 */
bool display_init(const aos_boot_params* params);

/**
 * 清屏。
 * 
 * @return 无返回值。
 */
void display_clear(void);

/**
 * 输出一个字符。
 * 
 * @param c 字符。
 * 
 * @return 无返回值。
 */
void display_putc(char32 c);

/**
 * 输出字符串。
 * 
 * @param text 字符串。
 * 
 * @return 无返回值。
 */
void display_puts(const char32* text);

/**
 * 格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 输出状态码。
 */
uintn display_printf(const char8* format,...);

#endif /*__AOS_KERNEL_INIT_DISPLAY_H__*/
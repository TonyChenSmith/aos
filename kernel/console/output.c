/**
 * 控制台输出及其初始化。
 * @date 2026-11-01
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <console/output.h>
#include <init/params.h>

/**
 * 初始化控制台调试端口输出。
 * 
 * @return 无返回值。
 */
void console_debugcon_init(void);

/**
 * 初始化控制台图形显示。
 * 
 * @param params 启动参数。
 * 
 * @return 成功初始化返回真。
 */
bool console_graphics_init(const aos_boot_params* params);

/**
 * 初始化控制台输出。
 * 
 * @param params 启动参数。
 * 
 * @return 成功初始化返回真。
 */
bool kernel_console_output_init(const aos_boot_params* params)
{
    console_debugcon_init();
    return console_graphics_init(params);
}

/**
 * 在控制台格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 屏幕输出状态码。
 */
uintn console_print(const char8* format,...)
{
    va_list args;

    va_start(args,format);
    console_debugcon_print_valist(format,args);
    va_end(args);

    va_start(args,format);
    uintn result=console_graphics_print_valist(format,args);
    va_end(args);

    return result;
}
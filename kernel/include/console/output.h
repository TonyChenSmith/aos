/**
 * 内核控制台输出。
 * @date 2026-11-01
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_CONSOLE_OUTPUT_H__
#define __AOS_KERNEL_CONSOLE_OUTPUT_H__

#include <support/type.h>
#include <support/varargs.h>

/**
 * 这一块主要描述控制台图形界面支持的ANSI转义序列。
 * 序列由ESC(0x1B)引导，本实现只识别"ESC["开头的控制序列（CSI），其余ESC序列整段忽略。
 * 参数为十进制数字，以分号分隔，最多8个，单个参数上限65535，超出部分被钳制。
 * 未列出的控制序列整段忽略，不会输出任何字符。
 * 
 * ESC[(n)m
 * 选择图形再现（SGR）。n为1到多个参数，支持内容如下：
 * 0          恢复默认前景色与背景色。
 * 30-37      设置前景色为标准8色。
 * 38;5;n     设置前景色为256色索引。
 * 38;2;r;g;b 设置前景色为真彩色。
 * 39         恢复默认前景色。
 * 40-47      设置背景色为标准8色。
 * 48;5;n     设置背景色为256色索引。
 * 48;2;r;g;b 设置背景色为真彩色。
 * 49         恢复默认背景色。
 * 90-97      设置前景色为亮色8色。
 * 100-107    设置背景色为亮色8色。
 * 其余n忽略。颜色只影响此后的输出，已显示的内容保持原色，不做重绘。
 * 
 * ESC[(r);(c)f
 * ESC[(r);(c)H
 * 定位光标。r为行值，c为列值。
 * 行列均从1开始。缺省或0按1处理，只给出r时列按1处理。
 * 超出屏幕范围时钳制到边界，不视为错误。
 *
 * ESC[(n)J
 * 擦除显示区域。n为模式。
 * n为2或3时整屏填充背景色，光标不动。
 * 其余n忽略。
 *
 * ESC[(n)K
 * 擦除行内区域。n为模式，有以下三种：
 * 0或缺省 从光标位置擦到行尾。
 * 1       从行首擦到光标位置，包含光标所在字符。
 * 2       擦除整行。
 * 其余n忽略。
 *
 * ESC[(n)A
 * 光标上移n行，缺省为1，超出上边界时停在首行。
 * 
 * ESC[(n)B
 * 光标下移n行，缺省为1，超出下边界时停在末行。
 * 
 * ESC[(n)C
 * 光标右移n列，缺省为1，超出右边界时停在行末。
 * 
 * ESC[(n)D
 * 光标左移n列，缺省为1，超出左边界时停在行首。
 * 
 * 注：
 * 1.光标移动只改变后续输出的起点，不擦除原有内容。
 * 2.擦除操作使用当前背景色填充。
 * 3.屏幕四周如有留白保持为黑色，不随背景色变化。
 */

/**
 * 在控制台格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 屏幕输出状态码。
 */
uintn console_print(const char8* format,...);

/**
 * 在控制台调试端口格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 输出状态码。
 */
uintn console_debugcon_print(const char8* format,...);

/**
 * 在控制台调试端口通过可变参数列表格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 输出状态码。
 */
uintn console_debugcon_print_valist(const char8* format,va_list args);

/**
 * 在控制台图形界面上格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 输出状态码。
 */
uintn console_graphics_print(const char8* format,...);

/**
 * 在控制台图形界面上通过可变参数列表格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 输出状态码。
 */
uintn console_graphics_print_valist(const char8* format,va_list args);

/**
 * 获取屏幕总行数。
 * 
 * @return 总行数。
 */
uintn get_screen_rows(void);

/**
 * 获取屏幕总列数。
 * 
 * @return 总列数。
 */
uintn get_screen_columns(void);

/**
 * 获取光标所在行。
 * 
 * @return 光标所在行。
 */
uintn get_cursor_row(void);

/**
 * 获取光标所在列。
 * 
 * @return 光标所在列。
 */
uintn get_cursor_column(void);

/**
 * 设置光标位置。
 * 
 * @param row    行。范围[0,rows-1]。
 * @param column 列。范围[0,columns]。
 * 
 * @return 设置成功返回真。
 */
bool set_cursor(uintn row,uintn column);

/**
 * 设置后续使用的前景色。
 * 
 * @param r 红色分量。
 * @param g 绿色分量。
 * @param b 蓝色分量。
 * 
 * @return 无返回值。
 */
void set_foreground(uint8 r,uint8 g,uint8 b);

/**
 * 设置后续使用的背景色。
 * 
 * @param r 红色分量。
 * @param g 绿色分量。
 * @param b 蓝色分量。
 * 
 * @return 无返回值。
 */
void set_background(uint8 r,uint8 g,uint8 b);

/**
 * 发起填充背景色的清屏并将光标恢复为(0,0)。
 * 
 * @return 无返回值。
 */
void clear_screen(void);

/**
 * 手动刷新屏幕。
 * 
 * @return 无返回值。
 */
void flush_screen(void);

#endif /*__AOS_KERNEL_CONSOLE_OUTPUT_H__*/
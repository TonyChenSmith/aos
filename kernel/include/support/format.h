/**
 * 内核格式化输出库函数。
 * @date 2026-04-09
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_FORMAT_H__
#define __AOS_KERNEL_SUPPORT_FORMAT_H__

#include "handle.h"
#include "string.h"
#include "varargs.h"

/**
 * 这一块主要描述一下格式化输出的格式化序列。其中字符均指Unicode码元。
 * %i 代表32位有符号整数。小于该位数应由调用者自行转换。
 * %I 代表64位有符号整数。
 * %u 代表32位无符号整数。小于该位数应由调用者自行转换。
 * %U 代表64位无符号整数。
 * %c 代表一个Unicode字符。对应为char32类型。char8和char16应自行转换。
 * %s 代表一个字符串。对应为const char8*类型。其他字符串应由调用者转换后再传递。
 * %h 代表32位无符号整数16进制小写输出。
 * %H 代表32位无符号整数16进制大写输出。
 * %x 代表64位无符号整数16进制小写输出。
 * %X 代表64位无符号整数16进制大写输出。
 * %n 代表有符号指针整型输出。
 * %N 代表无符号指针整型输出。
 * %p 代表无符号指针整型16进制输出，前导0补齐，字母小写。
 * %P 代表无符号指针整型16进制输出，前导0补齐，字母大写。
 * %b 代表32位无符号整数2进制输出。
 * %B 代表64位无符号整数2进制输出。
 * %% 代表%字符。
 * 
 * 下面为各格式化序列功能细节。
 * %[-][width][.precision](iIuUhHxXnNbB)
 * 正常是右对齐，前缀-代表左对齐。
 * 宽度默认是0，当输入0以外的正数后计算出新位宽，用于估计该格式化位置所占空间，包括符号位，当然空间不足还是会向右溢出。
 * 精度默认是1，仅保证显示一个0。当精度输入大于1的数值时保证数值的最少显示位数。精度显示要求必定满足。
 * 显示过程应该是先保证精度字符串，然后再检查宽度决定是否填充。
 * 
 * %(pP)
 * 指针整型，固定宽度，32位宽为8字符，64位位宽为16字符，保证前导0填充。
 * 
 * %[-][width][.length]s
 * 正常是右对齐，前缀-代表左对齐。
 * 宽度表示该输出的最小字符数，如果字符串长度不足就根据对齐补充空格。
 * 长度正数时表示可以显示的最长长度。
 * 
 * %(%c)
 * 输出一个字符。
 * 
 * 所有错误解析都会保留其结果进行运算，当然小于默认值则补足，要是未识别到已知格式化序列则放弃显示。
 */

/**
 * 字符串格式化输出到输入输出句柄。这里输出字符串按UTF-8编码。
 * 
 * @param handle 输入输出句柄。要求其可写能力。
 * @param format 格式化字符串。
 * 
 * @return 返回输出状态码。
 */
uint64 format_print(io_handle* handle,const char8* format,...);

/**
 * 字符串格式化输出到字符串缓存。这里输出字符串按UTF-8编码，保证缓存有结尾字符。
 * 
 * @param buffer 字符串缓存。
 * @param size   字符串缓存长度。包括存储结尾字符的长度。
 * @param format 格式化字符串。
 * 
 * @return 已写入长度，包括结尾0。
 */
uintn format_string(char8* buffer,uintn size,const char8* format,...);

/**
 * 字符串格式化输出到输入输出句柄，参数通过可变参数列表提供。这里输出字符串按UTF-8编码。
 * 
 * @param handle 输入输出句柄。要求其可写能力。
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 返回输出状态码。
 */
uintn format_print_valist(io_handle* handle,const char8* format,va_list args);

/**
 * 字符串格式化输出到字符串缓存，参数通过可变参数列表提供。这里输出字符串按UTF-8编码，保证缓存有结尾字符。
 * 
 * @param buffer 字符串缓存。
 * @param size   字符串缓存长度。包括存储结尾字符的长度。
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 已写入长度，包括结尾0。
 */
uintn format_string_valist(char8* buffer,uintn size,const char8* format,va_list args);

#endif /*__AOS_KERNEL_SUPPORT_FORMAT_H__*/
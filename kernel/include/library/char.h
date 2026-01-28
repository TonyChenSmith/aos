/**
 * 内核字符操作库函数。
 * @date 2025-12-25
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIBRARY_CHAR_H__
#define __AOS_KERNEL_LIBRARY_CHAR_H__

#include "ktype.h"

/**
 * 判断输入字符是否为ASCII字母。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_alphabetic(char32 c);

/**
 * 判断输入字符是否为ASCII空白分隔符。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_blank(char32 c);

/**
 * 判断输入字符是否为ASCII控制字符。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_control(char32 c);

/**
 * 判断输入字符是否为ASCII数字。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_digit(char32 c);

/**
 * 判断输入字符是否为ASCII小写字母。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_lowercase(char32 c);

/**
 * 判断输入字符是否为ASCII大写字母。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_uppercase(char32 c);

/**
 * 判断输入字符是否为ASCII空白字符。
 * 
 * @param c 输入字符。
 * 
 * @return 如果在范围内返回真。
 */
bool ascii_is_whitespace(char32 c);

/**
 * 将输入的大写字符转换成小写字符。
 * 
 * @param c 输入字符。
 * 
 * @return 输入字符对应小写字符，如果不是可转换字符返回原值。
 */
char32 to_lowercase(char32 c);

/**
 * 将输入的小写字符转换成大写字符。
 * 
 * @param c 输入字符。
 * 
 * @return 输入字符对应大写字符，如果不是可转换字符返回原值。
 */
char32 to_uppercase(char32 c);

#endif /*__AOS_KERNEL_LIBRARY_CHAR_H__*/
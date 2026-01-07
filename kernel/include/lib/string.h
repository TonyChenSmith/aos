/**
 * 内核字符串操作库函数。
 * @date 2025-12-24
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_STRING_H__
#define __AOS_KERNEL_LIB_STRING_H__

#include <lib/kdef.h>
#include <lib/ktype.h>

/**
 * 获取字符串长度。
 */
#define string_length(s) (generic((s),char8*:string_length8,const char8*:string_length8,char16*:string_length16,\
    const char16*:string_length16,char32*:string_length32,const char32*:string_length32)(s))

/**
 * 获取字符串码点数。
 */
#define string_codepoint_count(s) (generic((s),char8*:string_codepoint_count8,const char8*:string_codepoint_count8,\
    char16*:string_codepoint_count16,const char16*:string_codepoint_count16,char32*:string_codepoint_count32,\
    const char32*:string_codepoint_count32)(s))

/**
 * 检查字符串编码有效。
 */
#define string_valid(s) (generic((s),char8*:string_valid8,const char8*:string_valid8,char16*:string_valid16,\
    const char16*:string_valid16,char32*:string_valid32,const char32*:string_valid32)(s))

/**
 * 获取8位字符串长度。
 * 
 * @param s 8位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length8(const char8* s);

/**
 * 获取16位字符串长度。
 * 
 * @param s 16位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length16(const char16* s);

/**
 * 获取32位字符串长度。
 * 
 * @param s 32位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length32(const char32* s);

/**
 * 获取8位字符串码点数。
 * 
 * @param s 8位字符串。
 * 
 * @return 字符串到终结字符前码点数目，非法编码返回0。
 */
uintn string_codepoint_count8(const char8* s);

/**
 * 获取16位字符串码点数。
 * 
 * @param s 16位字符串。
 * 
 * @return 字符串到终结字符前码点数目，非法编码返回0。
 */
uintn string_codepoint_count16(const char16* s);

/**
 * 获取32位字符串码点数。
 * 
 * @param s 32位字符串。
 * 
 * @return 字符串到终结字符前码点数目，非法编码返回0。
 */
uintn string_codepoint_count32(const char32* s);

/**
 * 检查8位字符串编码有效。
 * 
 * @param s 8位字符串。
 * 
 * @return 编码有效返回真。
 */
static inline bool string_valid8(const char8* s)
{
    return string_codepoint_count8(s)!=0;
}

/**
 * 检查16位字符串编码有效。
 * 
 * @param s 16位字符串。
 * 
 * @return 编码有效返回真。
 */
static inline bool string_valid16(const char16* s)
{
    return string_codepoint_count16(s)!=0;
}

/**
 * 检查32位字符串编码有效。
 * 
 * @param s 32位字符串。
 * 
 * @return 编码有效返回真。
 */
static inline bool string_valid32(const char32* s)
{
    return string_codepoint_count32(s)!=0;
}

#endif /*__AOS_KERNEL_LIB_STRING_H__*/
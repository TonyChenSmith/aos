/**
 * 内核字符串操作库函数。
 * @date 2025-12-24
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_STRING_H__
#define __AOS_KERNEL_SUPPORT_STRING_H__

#include "kdef.h"
#include "ktype.h"

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
 * 字符串转换成8位字符串。
 */
#define string_convert8(dest,src,size) (generic((src),char8*:string_convert8_char8,const char8*:string_convert8_char8,\
    char16*:string_convert8_char16,const char16*:string_convert8_char16,char32*:string_convert8_char32,\
    const char32*:string_convert8_char32)(dest,src,size))

/**
 * 字符串转换成16位字符串。
 */
#define string_convert16(dest,src,size) (generic((src),char8*:string_convert16_char8,\
    const char8*:string_convert16_char8,char16*:string_convert16_char16,const char16*:string_convert16_char16,\
    char32*:string_convert16_char32,const char32*:string_convert16_char32)(dest,src,size))

/**
 * 字符串转换成32位字符串。
 */
#define string_convert32(dest,src,size) (generic((src),char8*:string_convert32_char8,\
    const char8*:string_convert32_char8,char16*:string_convert32_char16,const char16*:string_convert32_char16,\
    char32*:string_convert32_char32,const char32*:string_convert32_char32)(dest,src,size))

/**
 * 连接字符串b到字符串a后面，需要字符串a的可用空间大小。
 */
#define string_concat(a,b,size) (generic((a),char8*:string_concat8,char16*:string_concat16,\
    char32*:string_concat32)(a,b,size))

/**
 * 复制源字符串到目标字符串，需要源字符串的可用空间大小。
 */
#define string_copy(dest,src,size) (generic((dest),char8*:string_copy8,char16*:string_copy16,\
    char32*:string_copy32)(dest,src,size))

/**
 * 搜索源字符串中从前往后第一次出现目标字符串的位置。
 */
#define string_find(src,target) (generic((src),char8*:string_find8,const char8*:string_find8,char16*:string_find16,\
    const char16*:string_find16,char32*:string_find32,const char32*:string_find32)(src,target))

/**
 * 搜索源字符串中从后往前第一次出现目标字符串的位置。
 */
#define string_reverse_find(src,target) (generic((src),char8*:string_reverse_find8,const char8*:string_reverse_find8,\
    char16*:string_reverse_find16,const char16*:string_reverse_find16,char32*:string_reverse_find32,\
    const char32*:string_reverse_find32)(src,target))

/**
 * 搜索字符串中从前往后第一次出现目标字符的位置。
 */
#define string_find_char(s,c) (generic((s),char8*:string_find_char8,const char8*:string_find_char8,\
    char16*:string_find_char16,const char16*:string_find_char16,char32*:string_find_char32,\
    const char32*:string_find_char32)(s,c))

/**
 * 搜索字符串中从后往前第一次出现目标字符的位置。
 */
#define string_reverse_find_char(s,c) (generic((s),char8*:string_reverse_find_char8,\
    const char8*:string_reverse_find_char8,char16*:string_reverse_find_char16,const char16*:string_reverse_find_char16,\
    char32*:string_reverse_find_char32,const char32*:string_reverse_find_char32)(s,c))

/**
 * 对字符串a与字符串b进行比较。
 */
#define string_compare(a,b) (generic((a),char8*:string_compare8,const char8*:string_compare8,\
    char16*:string_compare16,const char16*:string_compare16,char32*:string_compare32,\
    const char32*:string_compare32)(a,b))

/**
 * 从字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 */
#define string_substring(dest,src,start,length,size) (generic((dest),char8*:string_substring8,\
    char16*:string_substring16,char32*:string_substring32)(dest,src,start,length,size))

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
 * @return 字符串到终结字符前码点数目，无效编码返回0。
 */
uintn string_codepoint_count8(const char8* s);

/**
 * 获取16位字符串码点数。
 * 
 * @param s 16位字符串。
 * 
 * @return 字符串到终结字符前码点数目，无效编码返回0。
 */
uintn string_codepoint_count16(const char16* s);

/**
 * 获取32位字符串码点数。
 * 
 * @param s 32位字符串。
 * 
 * @return 字符串到终结字符前码点数目，无效编码返回0。
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
    return s[0]==0||string_codepoint_count8(s)!=0;
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
    return s[0]==0||string_codepoint_count16(s)!=0;
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
    return s[0]==0||string_codepoint_count32(s)!=0;
}

/**
 * 将8位字符串转换成等价有效8位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert8_char8(char8* dest,const char8* src,uintn size);

/**
 * 将16位字符串转换成8位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  16位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert8_char16(char8* dest,const char16* src,uintn size);

/**
 * 将32位字符串转换成8位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  32位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert8_char32(char8* dest,const char32* src,uintn size);

/**
 * 将8位字符串转换成16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert16_char8(char16* dest,const char8* src,uintn size);

/**
 * 将16位字符串转换成等价有效16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  16位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert16_char16(char16* dest,const char16* src,uintn size);

/**
 * 将32位字符串转换成16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  32位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert16_char32(char16* dest,const char32* src,uintn size);

/**
 * 将8位字符串转换成32位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert32_char8(char32* dest,const char8* src,uintn size);

/**
 * 将16位字符串转换成32位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  16位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert32_char16(char32* dest,const char16* src,uintn size);

/**
 * 将32位字符串转换成等价有效32位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  32位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert32_char32(char32* dest,const char32* src,uintn size);

/**
 * 连接8位字符串b到字符串a后面，需要字符串a的可用空间大小。
 * 
 * @param a    字符串a。
 * @param b    字符串b。
 * @param size 包含字符串a预留空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_concat8(char8* a,const char8* b,uintn size);

/**
 * 连接16位字符串b到字符串a后面，需要字符串a的可用空间大小。
 * 
 * @param a    字符串a。
 * @param b    字符串b。
 * @param size 包含字符串a预留空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_concat16(char16* a,const char16* b,uintn size);

/**
 * 连接32位字符串b到字符串a后面，需要字符串a的可用空间大小。
 * 
 * @param a    字符串a。
 * @param b    字符串b。
 * @param size 包含字符串a预留空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_concat32(char32* a,const char32* b,uintn size);

/**
 * 复制8位的源字符串到目标字符串，需要源字符串的可用空间大小。
 * 
 * @param dest 目标字符串。
 * @param src  源字符串。
 * @param size 目标字符串可用空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_copy8(char8* dest,const char8* src,uintn size);

/**
 * 复制16位的源字符串到目标字符串，需要源字符串的可用空间大小。
 * 
 * @param dest 目标字符串。
 * @param src  源字符串。
 * @param size 目标字符串可用空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_copy16(char16* dest,const char16* src,uintn size);

/**
 * 复制32位的源字符串到目标字符串，需要源字符串的可用空间大小。
 * 
 * @param dest 目标字符串。
 * @param src  源字符串。
 * @param size 目标字符串可用空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_copy32(char32* dest,const char32* src,uintn size);

/**
 * 搜索8位源字符串中从前往后第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_find8(const char8* src,const char8* target);

/**
 * 搜索16位源字符串中从前往后第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_find16(const char16* src,const char16* target);

/**
 * 搜索32位源字符串中从前往后第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_find32(const char32* src,const char32* target);

/**
 * 搜索8位源字符串中从后往前第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_reverse_find8(const char8* src,const char8* target);

/**
 * 搜索16位源字符串中从后往前第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_reverse_find16(const char16* src,const char16* target);

/**
 * 搜索32位源字符串中从后往前第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_reverse_find32(const char32* src,const char32* target);

/**
 * 搜索8位字符串中从前往后第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_find_char8(const char8* s,char8 c);

/**
 * 搜索16位字符串中从前往后第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_find_char16(const char16* s,char16 c);

/**
 * 搜索32位字符串中从前往后第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_find_char32(const char32* s,char32 c);

/**
 * 搜索8位字符串中从后往前第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_reverse_find_char8(const char8* s,char8 c);

/**
 * 搜索16位字符串中从后往前第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_reverse_find_char16(const char16* s,char16 c);

/**
 * 搜索32位字符串中从后往前第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_reverse_find_char32(const char32* s,char32 c);

/**
 * 对8位字符串a与字符串b进行比较。
 * 
 * @param a 字符串a。
 * @param b 字符串b。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 string_compare8(const char8* a,const char8* b);

/**
 * 对16位字符串a与字符串b进行比较。
 * 
 * @param a 字符串a。
 * @param b 字符串b。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 string_compare16(const char16* a,const char16* b);

/**
 * 对32位字符串a与字符串b进行比较。
 * 
 * @param a 字符串a。
 * @param b 字符串b。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 string_compare32(const char32* a,const char32* b);

/**
 * 从8位字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 * 
 * @param dest   目标字符串。
 * @param src    源字符串。
 * @param start  起始索引。
 * @param length 获取长度。
 * @param size   目标字符串可用大小，包含终结符。
 * 
 * @return 能获取返回一个正数长度，不能获取返回0。
 */
uintn string_substring8(char8* dest,const char8* src,uintn start,uintn length,uintn size);

/**
 * 从16位字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 * 
 * @param dest   目标字符串。
 * @param src    源字符串。
 * @param start  起始索引。
 * @param length 获取长度。
 * @param size   目标字符串可用大小，包含终结符。
 * 
 * @return 能获取返回一个正数长度，不能获取返回0。
 */
uintn string_substring16(char16* dest,const char16* src,uintn start,uintn length,uintn size);

/**
 * 从32位字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 * 
 * @param dest   目标字符串。
 * @param src    源字符串。
 * @param start  起始索引。
 * @param length 获取长度。
 * @param size   目标字符串可用大小，包含终结符。
 * 
 * @return 能获取返回一个正数长度，不能获取返回0。
 */
uintn string_substring32(char32* dest,const char32* src,uintn start,uintn length,uintn size);

#endif /*__AOS_KERNEL_SUPPORT_STRING_H__*/
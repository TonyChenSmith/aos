/**
 * 字体解析器使用数据类型，这里是为了与内核模块统一。
 * @date 2026-02-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_RESEARCH_FONTPARSER_TYPE_H__
#define __AOS_RESEARCH_FONTPARSER_TYPE_H__

#include <stdint.h>

/**
 * 有符号8位整型。
 */
typedef int8_t int8;

/**
 * 有符号16位整型。
 */
typedef int16_t int16;

/**
 * 有符号32位整型。
 */
typedef int32_t int32;

/**
 * 有符号64位整型。
 */
typedef int64_t int64;

/**
 * 无符号8位整型。
 */
typedef uint8_t uint8;

/**
 * 无符号16位整型。
 */
typedef uint16_t uint16;

/**
 * 无符号32位整型。
 */
typedef uint32_t uint32;

/**
 * 无符号64位整型。
 */
typedef uint64_t uint64;

/**
 * 布尔类型。
 */
typedef uint8 bool;

/**
 * 8位字符类型。
 */
typedef char char8;

/**
 * 16位字符类型。
 */
typedef __CHAR16_TYPE__ char16;

/**
 * 32位字符类型。
 */
typedef __CHAR32_TYPE__ char32;

#ifdef __x86_64__

/**
 * 有符号指针整型。
 */
typedef int64 intn;

/**
 * 无符号指针整型。
 */
typedef uint64 uintn;

#else

/**
 * 有符号指针整型。
 */
typedef int32 intn;

/**
 * 无符号指针整型。
 */
typedef uint32 uintn;

#endif /*__x86_64__*/

#endif /*__AOS_RESEARCH_FONTPARSER_TYPE_H__*/
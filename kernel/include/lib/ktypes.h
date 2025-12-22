/**
 * 内核使用的基础数据类型。
 * @date 2025-12-22
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_KTYPES_H__
#define __AOS_KERNEL_LIB_KTYPES_H__

/**
 * 有符号8位整型。
 */
typedef __INT8_TYPE__ int8;

/**
 * 有符号16位整型。
 */
typedef __INT16_TYPE__ int16;

/**
 * 有符号32位整型。
 */
typedef __INT32_TYPE__ int32;

/**
 * 有符号64位整型。
 */
typedef __INT64_TYPE__ int64;

/**
 * 无符号8位整型。
 */
typedef __UINT8_TYPE__ uint8;

/**
 * 无符号16位整型。
 */
typedef __UINT16_TYPE__ uint16;

/**
 * 无符号32位整型。
 */
typedef __UINT32_TYPE__ uint32;

/**
 * 无符号64位整型。
 */
typedef __UINT64_TYPE__ uint64;

/**
 * 有符号指针整型。
 */
typedef __INTPTR_TYPE__ intn;

/**
 * 无符号指针整型。
 */
typedef __UINTPTR_TYPE__ uintn;

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
typedef int16 char16;

/**
 * 32位字符类型。
 */
typedef int32 char32;

#endif /*__AOS_KERNEL_LIB_KTYPES_H__*/
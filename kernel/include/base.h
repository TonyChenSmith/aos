/**
 * 内核使用的基础定义。包含内核类C库里需要的C标准库定义。
 * @date 2025-12-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_BASE_H__
#define __AOS_KERNEL_BASE_H__

/**
 * 功能关键字。
 */
#define asm __asm__
#define noreturn _Noreturn

/**
 * 有符号整型。
 */
typedef __INT8_TYPE__ int8;
typedef __INT16_TYPE__ int16;
typedef __INT32_TYPE__ int32;
typedef __INT64_TYPE__ int64;

/**
 * 无符号整型。
 */
typedef __UINT8_TYPE__ uint8;
typedef __UINT16_TYPE__ uint16;
typedef __UINT32_TYPE__ uint32;
typedef __UINT64_TYPE__ uint64;

/**
 * 指针整型。
 */
typedef __INTPTR_TYPE__ intn;
typedef __UINTPTR_TYPE__ uintn;

/**
 * 其他类型。字符类型为支持字面量选择了有符号类型。
 */
typedef uint8 bool;
typedef char char8;
typedef int16 char16;
typedef int32 char32;

/**
 * 整型取值极限。
 */
#define INT8_MAX __INT8_MAX__
#define INT16_MAX __INT16_MAX__
#define INT32_MAX __INT32_MAX__
#define INT64_MAX __INT64_MAX__

#define UINT8_MAX __UINT8_MAX__
#define UINT16_MAX __UINT16_MAX__
#define UINT32_MAX __UINT32_MAX__
#define UINT64_MAX __UINT64_MAX__

#define INTN_MAX __INTPTR_MAX__
#define UINTN_MAX __UINTPTR_MAX__

#define INT8_MIN (-INT8_MAX-1)
#define INT16_MIN (-INT16_MAX-1)
#define INT32_MIN (-INT32_MAX-1)
#define INT64_MIN (-INT64_MAX-1)

#define UINT8_MIN 0
#define UINT16_MIN 0
#define UINT32_MIN 0U
#define UINT64_MIN 0UL

#define INTN_MIN (-INTN_MAX-1)
#define UINTN_MIN 0UL

/**
 * 其他类型取值极限。
 */
#define true ((bool)(1==1))
#define false ((bool)(1==0))

#define CHAR8_MAX UINT8_MAX
#define CHAR16_MAX UINT16_MAX
#define CHAR32_MAX UINT32_MAX

#define CHAR8_MIN UINT8_MIN
#define CHAR16_MIN UINT16_MIN
#define CHAR32_MIN UINT32_MIN

/**
 * 位常量。
 */
#define BIT0 0x1
#define BIT1 0x2
#define BIT2 0x4
#define BIT3 0x8
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
#define BIT8 0x100
#define BIT9 0x200
#define BIT10 0x400
#define BIT11 0x800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x10000
#define BIT17 0x20000
#define BIT18 0x40000
#define BIT19 0x80000
#define BIT20 0x100000
#define BIT21 0x200000
#define BIT22 0x400000
#define BIT23 0x800000
#define BIT24 0x1000000
#define BIT25 0x2000000
#define BIT26 0x4000000
#define BIT27 0x8000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000
#define BIT32 __UINT64_C(0x100000000)
#define BIT33 __UINT64_C(0x200000000)
#define BIT34 __UINT64_C(0x400000000)
#define BIT35 __UINT64_C(0x800000000)
#define BIT36 __UINT64_C(0x1000000000)
#define BIT37 __UINT64_C(0x2000000000)
#define BIT38 __UINT64_C(0x4000000000)
#define BIT39 __UINT64_C(0x8000000000)
#define BIT40 __UINT64_C(0x10000000000)
#define BIT41 __UINT64_C(0x20000000000)
#define BIT42 __UINT64_C(0x40000000000)
#define BIT43 __UINT64_C(0x80000000000)
#define BIT44 __UINT64_C(0x100000000000)
#define BIT45 __UINT64_C(0x200000000000)
#define BIT46 __UINT64_C(0x400000000000)
#define BIT47 __UINT64_C(0x800000000000)
#define BIT48 __UINT64_C(0x1000000000000)
#define BIT49 __UINT64_C(0x2000000000000)
#define BIT50 __UINT64_C(0x4000000000000)
#define BIT51 __UINT64_C(0x8000000000000)
#define BIT52 __UINT64_C(0x10000000000000)
#define BIT53 __UINT64_C(0x20000000000000)
#define BIT54 __UINT64_C(0x40000000000000)
#define BIT55 __UINT64_C(0x80000000000000)
#define BIT56 __UINT64_C(0x100000000000000)
#define BIT57 __UINT64_C(0x200000000000000)
#define BIT58 __UINT64_C(0x400000000000000)
#define BIT59 __UINT64_C(0x800000000000000)
#define BIT60 __UINT64_C(0x1000000000000000)
#define BIT61 __UINT64_C(0x2000000000000000)
#define BIT62 __UINT64_C(0x4000000000000000)
#define BIT63 __UINT64_C(0x8000000000000000)

/**
 * 其他常量定义。
 */
#define NULL ((void*)0)

#endif /*__AOS_KERNEL_BASE_H__*/
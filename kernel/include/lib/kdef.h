/**
 * 内核使用的基础宏。
 * @date 2025-12-22
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_KDEF_H__
#define __AOS_KERNEL_LIB_KDEF_H__

#include "ktype.h"

/**
 * 查询对齐要求。
 */
#define alignof _Alignof

/**
 * 指定对齐方式。
 */
#define alignas _Alignas

/**
 * 泛型。
 */
#define generic _Generic

/**
 * 不返回。
 */
#define noreturn _Noreturn

/**
 * 逻辑真。
 */
#define true ((bool)(1==1))

/**
 * 逻辑假。
 */
#define false ((bool)(1==0))

/**
 * 空指针。
 */
#define null ((void*)0)

/**
 * 有符号8位整型最大值。
 */
#define INT8_MAX __INT8_MAX__

/**
 * 有符号16位整型最大值。
 */
#define INT16_MAX __INT16_MAX__

/**
 * 有符号32位整型最大值。
 */
#define INT32_MAX __INT32_MAX__

/**
 * 有符号64位整型最大值。
 */
#define INT64_MAX __INT64_MAX__

/**
 * 无符号8位整型最大值。
 */
#define UINT8_MAX __UINT8_MAX__

/**
 * 无符号16位整型最大值。
 */
#define UINT16_MAX __UINT16_MAX__

/**
 * 无符号32位整型最大值。
 */
#define UINT32_MAX __UINT32_MAX__

/**
 * 无符号64位整型最大值。
 */
#define UINT64_MAX __UINT64_MAX__

/**
 * 有符号8位整型最小值。
 */
#define INT8_MIN (-INT8_MAX-1)

/**
 * 有符号16位整型最小值。
 */
#define INT16_MIN (-INT16_MAX-1)

/**
 * 有符号32位整型最小值。
 */
#define INT32_MIN (-INT32_MAX-1)

/**
 * 有符号64位整型最小值。
 */
#define INT64_MIN (-INT64_MAX-1)

/**
 * 无符号8位整型最小值。
 */
#define UINT8_MIN ((uint8)0)

/**
 * 无符号16位整型最小值。
 */
#define UINT16_MIN ((uint16)0)

/**
 * 无符号32位整型最小值。
 */
#define UINT32_MIN ((uint32)0)

/**
 * 无符号64位整型最小值。
 */
#define UINT64_MIN ((uint64)0)

#ifdef __x86_64__

/**
 * 有符号指针整型最大值。
 */
#define INTN_MAX INT64_MAX

/**
 * 无符号指针整型最大值。
 */
#define UINTN_MAX UINT64_MAX

/**
 * 有符号指针整型最小值。
 */
#define INTN_MIN INT64_MIN

/**
 * 无符号指针整型最小值。
 */
#define UINTN_MIN UINT64_MIN

#else

/**
 * 有符号指针整型最大值。
 */
#define INTN_MAX INT32_MAX

/**
 * 无符号指针整型最大值。
 */
#define UINTN_MAX UINT32_MAX

/**
 * 有符号指针整型最小值。
 */
#define INTN_MIN INT32_MIN

/**
 * 无符号指针整型最小值。
 */
#define UINTN_MIN UINT32_MIN

#endif /*__x86_64__*/

/**
 * 位常量，覆盖0-63位。
 */
#define BIT0  0x1
#define BIT1  0x2
#define BIT2  0x4
#define BIT3  0x8
#define BIT4  0x10
#define BIT5  0x20
#define BIT6  0x40
#define BIT7  0x80
#define BIT8  0x100
#define BIT9  0x200
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
 * 大小常量。
 */
#define SIZE_1KB   0x400
#define SIZE_2KB   0x800
#define SIZE_4KB   0x1000
#define SIZE_8KB   0x2000
#define SIZE_16KB  0x4000
#define SIZE_32KB  0x8000
#define SIZE_64KB  0x10000
#define SIZE_128KB 0x20000
#define SIZE_256KB 0x40000
#define SIZE_512KB 0x80000
#define SIZE_1MB   0x100000
#define SIZE_2MB   0x200000
#define SIZE_4MB   0x400000
#define SIZE_8MB   0x800000
#define SIZE_16MB  0x1000000
#define SIZE_32MB  0x2000000
#define SIZE_64MB  0x4000000
#define SIZE_128MB 0x8000000
#define SIZE_256MB 0x10000000
#define SIZE_512MB 0x20000000
#define SIZE_1GB   0x40000000
#define SIZE_2GB   0x80000000
#define SIZE_4GB   __UINT64_C(100000000)
#define SIZE_8GB   __UINT64_C(200000000)
#define SIZE_16GB  __UINT64_C(400000000)
#define SIZE_32GB  __UINT64_C(800000000)
#define SIZE_64GB  __UINT64_C(1000000000)
#define SIZE_128GB __UINT64_C(2000000000)
#define SIZE_256GB __UINT64_C(4000000000)
#define SIZE_512GB __UINT64_C(8000000000)
#define SIZE_1TB   __UINT64_C(10000000000)
#define SIZE_2TB   __UINT64_C(20000000000)
#define SIZE_4TB   __UINT64_C(40000000000)
#define SIZE_8TB   __UINT64_C(80000000000)
#define SIZE_16TB  __UINT64_C(100000000000)
#define SIZE_32TB  __UINT64_C(200000000000)
#define SIZE_64TB  __UINT64_C(400000000000)
#define SIZE_128TB __UINT64_C(800000000000)
#define SIZE_256TB __UINT64_C(1000000000000)
#define SIZE_512TB __UINT64_C(2000000000000)
#define SIZE_1PB   __UINT64_C(4000000000000)
#define SIZE_2PB   __UINT64_C(8000000000000)
#define SIZE_4PB   __UINT64_C(10000000000000)
#define SIZE_8PB   __UINT64_C(20000000000000)
#define SIZE_16PB  __UINT64_C(40000000000000)
#define SIZE_32PB  __UINT64_C(80000000000000)
#define SIZE_64PB  __UINT64_C(100000000000000)
#define SIZE_128PB __UINT64_C(200000000000000)
#define SIZE_256PB __UINT64_C(400000000000000)
#define SIZE_512PB __UINT64_C(800000000000000)
#define SIZE_1EB   __UINT64_C(1000000000000000)
#define SIZE_2EB   __UINT64_C(2000000000000000)
#define SIZE_4EB   __UINT64_C(4000000000000000)
#define SIZE_8EB   __UINT64_C(8000000000000000)

#endif /*__AOS_KERNEL_LIB_KDEF_H__*/
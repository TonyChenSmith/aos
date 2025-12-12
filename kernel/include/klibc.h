/**
 * 内核可以使用的类C标准库函数。
 * @date 2025-12-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_KLIBC_H__
#define __AOS_KERNEL_KLIBC_H__

#include "base.h"

/**
 * string.h库函数实现。
 */

/**
 * 内存区域复制。
 * 
 * @param s1 目标基址。
 * @param s2 源基址。
 * @param n  复制字节数。
 * 
 * @return 复制后的目标基址。
 */
extern void* kmemcpy(void* restrict s1,const void* restrict s2,uintn n);

/**
 * 内存区域可重叠复制。
 * 
 * @param s1 目标基址。
 * @param s2 源基址。
 * @param n  复制字节数。
 * 
 * @return 复制后的目标基址。
 */
extern void* kmemmove(void* restrict s1,const void* restrict s2,uintn n);

#endif /*__AOS_KERNEL_KLIBC_H__*/
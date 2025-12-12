/**
 * 内核类C标准库字符串功能函数实现。
 * @date 2025-12-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <base.h>

/**
 * 内存区域复制。
 * 
 * @param s1 目标基址。
 * @param s2 源基址。
 * @param n  复制字节数。
 * 
 * @return 复制后的目标基址。
 */
extern void* kmemcpy(void* restrict s1,const void* restrict s2,uintn n)
{
    asm volatile("rep movsb":"+D"(s1),"+S"(s2),"+c"(n)::"memory");
    return s1;
}
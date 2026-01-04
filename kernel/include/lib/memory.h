/**
 * 内核内存操作库函数。
 * @date 2025-12-22
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_MEMORY_H__
#define __AOS_KERNEL_LIB_MEMORY_H__

#include "ktype.h"

/**
 * 将源内存复制前n个字节到目的内存。源内存与目的内在前n字节内有重叠时复制结果不保证正确。
 * 
 * @param dest 目的内存。
 * @param src  源内存。
 * @param n    复制字节数。
 * 
 * @return 返回目的内存的起始地址，便于进行链式处理。
 */
void* memory_copy(void* dest,const void* src,uintn n);

/**
 * 将源内存复制前n个字节到目的内存。这一函数允许复制时存在重叠情况。
 * 
 * @param dest 目的内存。
 * @param src  源内存。
 * @param n    复制字节数。
 * 
 * @return 返回目的内存的起始地址，便于进行链式处理。
 */
void* memory_move(void* dest,const void* src,uintn n);

/**
 * 将内存m前n个字节设置成固定值。
 * 
 * @param m     内存m。
 * @param value 设置值。
 * @param n     操作字节数。
 * 
 * @return 返回内存m的起始地址，便于进行链式处理。
 */
void* memory_set(void* m,uint8 value,uintn n);

/**
 * 对内存a与内存b的前n个字节进行比较。
 * 
 * @param a 内存a。
 * @param b 内存b。
 * @param n 比较字节数。
 * 
 * @return 相等返回真，不相等返回假。
 */
bool memory_compare(const void* a,const void* b,uintn n);

/**
 * 在内存m的前n个字节寻找输入字节第一次出现的位置。
 * 
 * @param m    内存m。
 * @param byte 被比较字节。
 * @param n    操作字节数。
 * 
 * @return 找到返回所在地址，未找到返回空指针。
 */
void* memory_find(const void* m,uint8 byte,uintn n);

/**
 * 将内存m前n个字节设置成0。
 * 
 * @param m     内存m。
 * @param n     操作字节数。
 * 
 * @return 返回内存m的起始地址，便于进行链式处理。
 */
static inline void* memory_zero(void* m,uintn n)
{
    return memory_set(m,0,n);
}

#endif /*__AOS_KERNEL_LIB_MEMORY_H__*/
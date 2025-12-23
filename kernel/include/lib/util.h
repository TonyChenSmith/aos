/**
 * 内核实用函数集合。
 * @date 2025-12-23
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_UTIL_H__
#define __AOS_KERNEL_LIB_UTIL_H__

/**
 * 取绝对值。
 */
#define abs(value) (((value)>0)?(value):(-(value)))

/**
 * 取最大值。
 */
#define max(a,b) (((a)>(b))?(a):(b))

/**
 * 取最小值。
 */
#define min(a,b) (((a)>(b))?(b):(a))

/**
 * 获取结构体字段在结构体内的偏移。
 */
#define offset_of(type,field) __builtin_offsetof(type,field)

/**
 * 8位位翻转。
 */
#define bit_reverse_8(value) __builtin_bitreverse8(value)

/**
 * 16位位翻转。
 */
#define bit_reverse_16(value) __builtin_bitreverse16(value)

/**
 * 32位位翻转。
 */
#define bit_reverse_32(value) __builtin_bitreverse32(value)

/**
 * 64位位翻转。
 */
#define bit_reverse_64(value) __builtin_bitreverse64(value)

/**
 * 8位循环左移。
 */
#define rotate_left_8(value,bits) __builtin_rotateleft8(value,bits)

/**
 * 16位循环左移。
 */
#define rotate_left_16(value,bits) __builtin_rotateleft16(value,bits)

/**
 * 32位循环左移。
 */
#define rotate_left_32(value,bits) __builtin_rotateleft32(value,bits)

/**
 * 64位循环左移。
 */
#define rotate_left_64(value,bits) __builtin_rotateleft64(value,bits)

/**
 * 8位循环右移。
 */
#define rotate_right_8(value,bits) __builtin_rotateright8(value,bits)

/**
 * 16位循环右移。
 */
#define rotate_right_16(value,bits) __builtin_rotateright16(value,bits)

/**
 * 32位循环右移。
 */
#define rotate_right_32(value,bits) __builtin_rotateright32(value,bits)

/**
 * 64位循环右移。
 */
#define rotate_right_64(value,bits) __builtin_rotateright64(value,bits)

/**
 * 计算数值中1的数目。
 */
#define popcount(value) __builtin_popcountg(value)

/**
 * 计算数值从最高有效位开始的连续0个数。如果value为0，返回fallback。
 */
#define clz(value,fallback) __builtin_clzg(value,fallback)

/**
 * 计算数值从最低有效位开始的连续0个数。如果value为0，返回fallback。
 */
#define ctz(value,fallback) __builtin_ctzg(value,fallback)

/**
 * 计算数值从最低有效位1的索引。如果value为0，返回fallback。
 */
#define ffs(value,fallback) ((value)==0?(fallback):ctz(value)+1)

/**
 * 判断一个值是否按给定的2的幂对齐。
 */
#define is_aligned(value,align) (((value)&((align)-1))==0)

/**
 * 数值按给定数向上对齐。
 */
#define align_up(value,align) ((value)+(((align)-(value))&((align)-1)))

#endif /*__AOS_KERNEL_LIB_UTIL_H__*/
/**
 * 内核实用函数集合。
 * @date 2025-12-23
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_UTIL_H__
#define __AOS_KERNEL_LIB_UTIL_H__

#include "kdef.h"
#include "ktype.h"

/**
 * 取绝对值。
 */
#define abs(value) (generic((value),int8:abs_int8,int16:abs_int16,int32:abs_int32,int64:abs_int64)(value))

/**
 * 取最大值。
 */
#define max(a,b) (generic((a+b),int8:max_int8,int16:max_int16,int32:max_int32,int64:max_int64,uint8:max_uint8,\
    uint16:max_uint16,uint32:max_uint32,uint64:max_uint64)(a,b))

/**
 * 取最小值。
 */
#define min(a,b) (generic((a+b),int8:min_int8,int16:min_int16,int32:min_int32,int64:min_int64,uint8:min_uint8,\
    uint16:min_uint16,uint32:min_uint32,uint64:min_uint64)(a,b))

/**
 * 获取结构体字段在结构体内的偏移。
 */
#define offset_of(type,field) __builtin_offsetof(type,field)

/**
 * 位反向。
 */
#define bit_reverse(value) (generic((value),uint8:bit_reverse_uint8,uint16:bit_reverse_uint16,uint32:bit_reverse_uint32,\
    uint64:bit_reverse_uint64)(value))

/**
 * 循环左移。
 */
#define rotate_left(value,bits) (generic((value),uint8:rotate_left_uint8,uint16:rotate_left_uint16,\
    uint32:rotate_left_uint32,uint64:rotate_left_uint64)(value,bits))

/**
 * 循环右移。
 */
#define rotate_right(value,bits) (generic((value),uint8:rotate_right_uint8,uint16:rotate_right_uint16,\
    uint32:rotate_right_uint32,uint64:rotate_right_uint64)(value,bits))

/**
 * 计算数值中1的数目。
 */
#define population_count(value) (generic((value),uint8:population_count_uint8,uint16:population_count_uint16,\
    uint32:population_count_uint32,uint64:population_count_uint64)(value))

/**
 * 计算数值从最高有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 */
#define count_leading_zeros(value) (generic((value),uint8:count_leading_zeros_uint8,uint16:count_leading_zeros_uint16,\
    uint32:count_leading_zeros_uint32,uint64:count_leading_zeros_uint64)(value))

/**
 * 计算数值从最低有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 */
#define count_trailing_zeros(value) (generic((value),uint8:count_trailing_zeros_uint8,\
    uint16:count_trailing_zeros_uint16,uint32:count_trailing_zeros_uint32,uint64:count_trailing_zeros_uint64)(value))

/**
 * 计算数值从最低有效位开始第一个1的索引。如果值为0，返回位数大小的索引。
 */
#define find_first_set(value) (generic((value),uint8:find_first_set_uint8,uint16:find_first_set_uint16,\
    uint32:find_first_set_uint32,uint64:find_first_set_uint64)(value))

/**
 * 取有符号8位整型的绝对值。
 * 
 * @param value 值。
 * 
 * @return 值的绝对值。
 */
static inline int8 abs_int8(int8 value)
{
    return value>0?value:-value;
}

/**
 * 取有符号8位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline int8 max_int8(int8 a,int8 b)
{
    return a>b?a:b;
}

/**
 * 取有符号8位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline int8 min_int8(int8 a,int8 b)
{
    return a>b?b:a;
}

/**
 * 取有符号16位整型的绝对值。
 * 
 * @param value 值。
 * 
 * @return 值的绝对值。
 */
static inline int16 abs_int16(int16 value)
{
    return value>0?value:-value;
}

/**
 * 取有符号16位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline int16 max_int16(int16 a,int16 b)
{
    return a>b?a:b;
}

/**
 * 取有符号16位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline int16 min_int16(int16 a,int16 b)
{
    return a>b?b:a;
}

/**
 * 取有符号32位整型的绝对值。
 * 
 * @param value 值。
 * 
 * @return 值的绝对值。
 */
static inline int32 abs_int32(int32 value)
{
    return value>0?value:-value;
}

/**
 * 取有符号32位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline int32 max_int32(int32 a,int32 b)
{
    return a>b?a:b;
}

/**
 * 取有符号32位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline int32 min_int32(int32 a,int32 b)
{
    return a>b?b:a;
}

/**
 * 取有符号64位整型的绝对值。
 * 
 * @param value 值。
 * 
 * @return 值的绝对值。
 */
static inline int64 abs_int64(int64 value)
{
    return value>0?value:-value;
}

/**
 * 取有符号64位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline int64 max_int64(int64 a,int64 b)
{
    return a>b?a:b;
}

/**
 * 取有符号64位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline int64 min_int64(int64 a,int64 b)
{
    return a>b?b:a;
}

/**
 * 取无符号8位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline uint8 max_uint8(uint8 a,uint8 b)
{
    return a>b?a:b;
}

/**
 * 取无符号8位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline uint8 min_uint8(uint8 a,uint8 b)
{
    return a>b?b:a;
}

/**
 * 取无符号16位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline uint16 max_uint16(uint16 a,uint16 b)
{
    return a>b?a:b;
}

/**
 * 取无符号16位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline uint16 min_uint16(uint16 a,uint16 b)
{
    return a>b?b:a;
}

/**
 * 取无符号32位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline uint32 max_uint32(uint32 a,uint32 b)
{
    return a>b?a:b;
}

/**
 * 取无符号32位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline uint32 min_uint32(uint32 a,uint32 b)
{
    return a>b?b:a;
}

/**
 * 取无符号64位整型的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最大值。
 */
static inline uint64 max_uint64(uint64 a,uint64 b)
{
    return a>b?a:b;
}

/**
 * 取无符号64位整型的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两者中的最小值。
 */
static inline uint64 min_uint64(uint64 a,uint64 b)
{
    return a>b?b:a;
}

/**
 * 对无符号8位整型位反向。
 * 
 * @param value 值。
 * 
 * @return 位反向值。
 */
static inline uint8 bit_reverse_uint8(uint8 value)
{
    return (uint8)__builtin_bitreverse8(value);
}

/**
 * 对无符号16位整型位反向。
 * 
 * @param value 值。
 * 
 * @return 位反向值。
 */
static inline uint16 bit_reverse_uint16(uint16 value)
{
    return (uint16)__builtin_bitreverse16(value);
}

/**
 * 对无符号32位整型位反向。
 * 
 * @param value 值。
 * 
 * @return 位反向值。
 */
static inline uint32 bit_reverse_uint32(uint32 value)
{
    return (uint32)__builtin_bitreverse32(value);
}

/**
 * 对无符号64位整型位反向。
 * 
 * @param value 值。
 * 
 * @return 位反向值。
 */
static inline uint64 bit_reverse_uint64(uint64 value)
{
    return (uint64)__builtin_bitreverse64(value);
}

/**
 * 对无符号8位整型循环左移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环左移后的值。
 */
static inline uint8 rotate_left_uint8(uint8 value,uintn bits)
{
    return (uint8)__builtin_rotateleft8(value,bits);
}

/**
 * 对无符号16位整型循环左移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环左移后的值。
 */
static inline uint16 rotate_left_uint16(uint16 value,uintn bits)
{
    return (uint16)__builtin_rotateleft16(value,bits);
}

/**
 * 对无符号32位整型循环左移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环左移后的值。
 */
static inline uint32 rotate_left_uint32(uint32 value,uintn bits)
{
    return (uint32)__builtin_rotateleft32(value,bits);
}

/**
 * 对无符号64位整型循环左移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环左移后的值。
 */
static inline uint64 rotate_left_uint64(uint64 value,uintn bits)
{
    return (uint64)__builtin_rotateleft64(value,bits);
}

/**
 * 对无符号8位整型循环右移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环右移后的值。
 */
static inline uint8 rotate_right_uint8(uint8 value,uintn bits)
{
    return (uint8)__builtin_rotateright8(value,bits);
}

/**
 * 对无符号16位整型循环右移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环右移后的值。
 */
static inline uint16 rotate_right_uint16(uint16 value,uintn bits)
{
    return (uint16)__builtin_rotateright16(value,bits);
}

/**
 * 对无符号32位整型循环右移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环右移后的值。
 */
static inline uint32 rotate_right_uint32(uint32 value,uintn bits)
{
    return (uint32)__builtin_rotateright32(value,bits);
}

/**
 * 对无符号64位整型循环右移。
 * 
 * @param value 值。
 * @param bits  位数。
 * 
 * @return 循环右移后的值。
 */
static inline uint64 rotate_right_uint64(uint64 value,uintn bits)
{
    return (uint64)__builtin_rotateright64(value,bits);
}

/**
 * 计算无符号8位整型中1的数目。
 * 
 * @param value 值。
 * 
 * @return 值中1的数目。
 */
static inline uintn population_count_uint8(uint8 value)
{
    return (uintn)__builtin_popcountg(value);
}

/**
 * 计算无符号16位整型中1的数目。
 * 
 * @param value 值。
 * 
 * @return 值中1的数目。
 */
static inline uintn population_count_uint16(uint16 value)
{
    return (uintn)__builtin_popcountg(value);
}

/**
 * 计算无符号32位整型中1的数目。
 * 
 * @param value 值。
 * 
 * @return 值中1的数目。
 */
static inline uintn population_count_uint32(uint32 value)
{
    return (uintn)__builtin_popcountg(value);
}

/**
 * 计算无符号64位整型中1的数目。
 * 
 * @param value 值。
 * 
 * @return 值中1的数目。
 */
static inline uintn population_count_uint64(uint64 value)
{
    return (uintn)__builtin_popcountg(value);
}

/**
 * 计算数值从最高有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最高有效位开始的连续0个数。
 */
static inline uintn count_leading_zeros_uint8(uint8 value)
{
    return (uintn)(value==0?8:__builtin_clzg(value));
}

/**
 * 计算数值从最高有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最高有效位开始的连续0个数。
 */
static inline uintn count_leading_zeros_uint16(uint16 value)
{
    return (uintn)(value==0?16:__builtin_clzg(value));
}

/**
 * 计算数值从最高有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最高有效位开始的连续0个数。
 */
static inline uintn count_leading_zeros_uint32(uint32 value)
{
    return (uintn)(value==0?32:__builtin_clzg(value));
}

/**
 * 计算数值从最高有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最高有效位开始的连续0个数。
 */
static inline uintn count_leading_zeros_uint64(uint64 value)
{
    return (uintn)(value==0?64:__builtin_clzg(value));
}

/**
 * 计算数值从最低有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始的连续0个数。
 */
static inline uintn count_trailing_zeros_uint8(uint8 value)
{
    return (uintn)(value==0?8:__builtin_ctzg(value));
}

/**
 * 计算数值从最低有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始的连续0个数。
 */
static inline uintn count_trailing_zeros_uint16(uint16 value)
{
    return (uintn)(value==0?16:__builtin_ctzg(value));
}

/**
 * 计算数值从最低有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始的连续0个数。
 */
static inline uintn count_trailing_zeros_uint32(uint32 value)
{
    return (uintn)(value==0?32:__builtin_ctzg(value));
}

/**
 * 计算数值从最低有效位开始的连续0个数。如果值为0，返回位数大小的0数目。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始的连续0个数。
 */
static inline uintn count_trailing_zeros_uint64(uint64 value)
{
    return (uintn)(value==0?64:__builtin_ctzg(value));
}

/**
 * 计算数值从最低有效位开始第一个1的索引。如果值为0，返回位数大小的索引。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始第一个1的索引。
 */
static inline uintn find_first_set_uint8(uint8 value)
{
    return (uintn)(value==0?8:count_trailing_zeros_uint8(value)+1);
}

/**
 * 计算数值从最低有效位开始第一个1的索引。如果值为0，返回位数大小的索引。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始第一个1的索引。
 */
static inline uintn find_first_set_uint16(uint16 value)
{
    return (uintn)(value==0?16:count_trailing_zeros_uint16(value)+1);
}


/**
 * 计算数值从最低有效位开始第一个1的索引。如果值为0，返回位数大小的索引。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始第一个1的索引。
 */
static inline uintn find_first_set_uint32(uint32 value)
{
    return (uintn)(value==0?32:count_trailing_zeros_uint32(value)+1);
}

/**
 * 计算数值从最低有效位开始第一个1的索引。如果值为0，返回位数大小的索引。
 * 
 * @param value 值。
 * 
 * @return 值中从最低有效位开始第一个1的索引。
 */
static inline uintn find_first_set_uint64(uint64 value)
{
    return (uintn)(value==0?64:count_trailing_zeros_uint64(value)+1);
}

/**
 * 判断一个值是否按给定对齐值对齐。
 * 
 * @param value 值。
 * @param align 对齐值。
 * 
 * @return 对齐返回真。
 */
static inline bool is_aligned(uintn value,uintn align)
{
    return value%align==0;
}

/**
 * 数值按给定对齐值向上对齐。
 * 
 * @param value 值。
 * @param align 对齐值。
 * 
 * @return 对齐返回真。
 */
static inline bool align_up(uintn value,uintn align)
{
    return (value/align)*align+(value%align==0?0:align);
}

#endif /*__AOS_KERNEL_LIB_UTIL_H__*/
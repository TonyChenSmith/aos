/**
 * 内核定点数库函数。
 * @date 2026-02-10
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_FIXED_H__
#define __AOS_KERNEL_SUPPORT_FIXED_H__

#include "ktype.h"
#include "util.h"

/**
 * Q15.16格式定点数。
 */
typedef int32 fixed32;

/**
 * Q31.32格式定点数。
 */
typedef int64 fixed64;

/**
 * 将整数和分数部分打包成32位定点数。
 * 计算方式为：fixed32=base+offset/0x10000。
 * 因此负数-1.5的调用参数组合应该为(-2,0x8000)。
 * 
 * @param base   基数部分，带符号。
 * @param offset 偏移部分。其偏移的分数值会加到基数部分。
 * 
 * @return 32位定点数。
 */
static inline fixed32 pack_fixed32(int16 base,uint16 offset)
{
    return ((fixed32)base)<<16|offset;
}

/**
 * 将整数和分数部分打包成64位定点数。
 * 计算方式为：fixed64=base+fraction/0x100000000。
 * 因此负数-1.5的调用参数组合应该为(-2,0x80000000)。
 * 
 * @param base   基数部分，带符号。
 * @param offset 偏移部分。其偏移的分数值会加到基数部分。
 * 
 * @return 64位定点数。
 */
static inline fixed64 pack_fixed64(int32 base,uint32 offset)
{
    return ((fixed64)base)<<32|offset;
}

/**
 * 按照定点数字面量创建32位定点数。
 * 
 * @param neg      是否为负数。
 * @param integer  整数字面量，仅正数范围，负数会转换成其绝对值（最小值除外）。
 * @param fraction 分数字面量。
 * 
 * @return 32位定点数。
 */
static inline fixed32 make_fixed32(bool neg,int16 integer,uint16 fraction)
{
    integer=abs_int16(integer);
    return neg?-pack_fixed32(integer,fraction):pack_fixed32(integer,fraction);
}

/**
 * 按照定点数字面量创建64位定点数。
 * 
 * @param neg      是否为负数。
 * @param integer  整数字面量，仅正数范围，负数会转换成其绝对值（最小值除外）。
 * @param fraction 分数字面量。
 * 
 * @return 64位定点数。
 */
static inline fixed64 make_fixed64(bool neg,int32 integer,uint32 fraction)
{
    integer=abs_int32(integer);
    return neg?-pack_fixed64(integer,fraction):pack_fixed64(integer,fraction);
}

/**
 * 将32位定点数扩展成64位定点数。
 * 
 * @param fixed 32位定点数。
 * 
 * @return 64位定点数。
 */
static inline fixed64 expand_fixed32(fixed32 fixed)
{
    return pack_fixed64(fixed>>16,(uint32)fixed<<16);
}

/**
 * 将64位定点数四舍五入成32位定点数。整数部分应由调用者自己保证不会被截断。
 * 
 * @param fixed 64位定点数。
 * 
 * @return 32位定点数。
 */
static inline fixed32 round_fixed64(fixed64 fixed)
{
    fixed+=0x8000;
    return pack_fixed32((int16)(fixed>>32),(uint16)(fixed>>16));
}

/**
 * 获取32位定点数内整数字面值。
 * 返回的值代表的是该定点数在字面含义的带符号整数部分，如-1.5的整数字面值为-1，而非打包函数使用的-2。
 * 关于整数字面值，定点数与其取负数值在整数字面值仅有符号区别。
 * 
 * @param fixed 32位定点数。
 * 
 * @return 整数字面值。
 */
static inline int16 fixed_integer32(fixed32 fixed)
{
    return (int16)(fixed<0?(fixed>>16)+1:(fixed>>16));
}

/**
 * 获取64位定点数内整数字面值。
 * 返回的值代表的是该定点数在字面含义的带符号整数部分，如-1.5的整数字面值为-1，而非打包函数使用的-2。
 * 关于整数字面值，定点数与其取负数值在整数字面值仅有符号区别。
 * 
 * @param fixed 64位定点数。
 * 
 * @return 整数字面值。
 */
static inline int32 fixed_integer64(fixed64 fixed)
{
    return (int32)(fixed<0?(fixed>>32)+1:(fixed>>32));
}

/**
 * 获取32位定点数内分数字面值。
 * 返回的值代表的是该定点数在字面含义的分数部分，如-1.5的分数字面值为0.5。
 * 关于分数字面值，定点数与其取负数值在分数字面值与正定点数的字面值一致。
 * 
 * @param fixed 32位定点数。
 * 
 * @return 分数字面值。
 */
static inline uint16 fixed_fraction32(fixed32 fixed)
{
    return (uint16)(fixed<0?-fixed&UINT16_MAX:fixed&UINT16_MAX);
}

/**
 * 获取64位定点数内分数字面值。
 * 返回的值代表的是该定点数在字面含义的分数部分，如-1.5的分数字面值为0.5。
 * 关于分数字面值，定点数与其取负数值在分数字面值与正定点数的字面值一致。
 * 
 * @param fixed 64位定点数。
 * 
 * @return 分数字面值。
 */
static inline uint32 fixed_fraction64(fixed64 fixed)
{
    return (uint32)(fixed<0?-fixed&UINT32_MAX:fixed&UINT32_MAX);
}

/**
 * 计数32位定点数a和b的和。
 * 
 * @param a 32位定点数a。
 * @param b 32位定点数b。
 * 
 * @return 定点数a和b的和。
 */
static inline fixed32 fixed_add32(fixed32 a,fixed32 b)
{
    return a+b;
}

/**
 * 计数64位定点数a和b的和。
 * 
 * @param a 64位定点数a。
 * @param b 64位定点数b。
 * 
 * @return 定点数a和b的和。
 */
static inline fixed64 fixed_add64(fixed64 a,fixed64 b)
{
    return a+b;
}

/**
 * 计数32位定点数a减b的差。
 * 
 * @param a 32位定点数a。
 * @param b 32位定点数b。
 * 
 * @return 定点数a减b的差。
 */
static inline fixed32 fixed_sub32(fixed32 a,fixed32 b)
{
    return a-b;
}

/**
 * 计数64位定点数a减b的差。
 * 
 * @param a 64位定点数a。
 * @param b 64位定点数b。
 * 
 * @return 定点数a减b的差。
 */
static inline fixed64 fixed_sub64(fixed64 a,fixed64 b)
{
    return a-b;
}

/**
 * 计数32位定点数a和b的积。
 * 
 * @param a 32位定点数a。
 * @param b 32位定点数b。
 * 
 * @return 定点数a和b的积。
 */
static inline fixed32 fixed_mul32(fixed32 a,fixed32 b)
{
    return (fixed32)((fixed64)a*b>>16);
}

/**
 * 计数64位定点数a和b的积。
 * 
 * @param a 64位定点数a。
 * @param b 64位定点数b。
 * 
 * @return 定点数a和b的积。
 */
static inline fixed64 fixed_mul64(fixed64 a,fixed64 b)
{
    return (fixed64)((__int128)a*b>>32);
}

/**
 * 计数32位定点数a除以b的商。
 * 
 * @param a 32位定点数a。
 * @param b 32位定点数b。
 * 
 * @return 定点数a除以b的商。
 */
static inline fixed32 fixed_div32(fixed32 a,fixed32 b)
{
    return (fixed32)(((fixed64)a<<16)/b);
}

/**
 * 计数64位定点数a除以b的商。
 * 
 * @param a 64位定点数a。
 * @param b 64位定点数b。
 * 
 * @return 定点数a除以b的商。
 */
static inline fixed64 fixed_div64(fixed64 a,fixed64 b)
{
    __asm__ volatile(
        "movq %%rax,%%rdx\n\t"
        "sarq $32,%%rdx\n\t"
        "shlq $32,%%rax\n\t"
        "idivq %1"
        :"+a"(a)
        :"r"(b)
        :"%rdx","cc"
    );
    return a;
}

/**
 * 取32位定点数的绝对值。
 * 
 * @param fixed 32位定点数。
 * 
 * @return 定点数的绝对值，除了最小值还是最小值。
 */
static inline fixed32 fixed_abs32(fixed32 fixed)
{
    return abs_int32(fixed);
}

/**
 * 取64位定点数的绝对值。
 * 
 * @param fixed 64位定点数。
 * 
 * @return 定点数的绝对值，除了最小值还是最小值。
 */
static inline fixed64 fixed_abs64(fixed64 fixed)
{
    return abs_int64(fixed);
}

/**
 * 取32位定点数的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两个定点数中的最大值。
 */
static inline fixed32 fixed_max32(fixed32 a,fixed32 b)
{
    return max_int32(a,b);
}

/**
 * 取64位定点数的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两个定点数中的最大值。
 */
static inline fixed64 fixed_max64(fixed64 a,fixed64 b)
{
    return max_int64(a,b);
}

/**
 * 取32位定点数的最小值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两个定点数中的最小值。
 */
static inline fixed32 fixed_min32(fixed32 a,fixed32 b)
{
    return min_int32(a,b);
}

/**
 * 取64位定点数的最大值。
 * 
 * @param a 左值。
 * @param b 右值。
 * 
 * @return 两个定点数中的最小值。
 */
static inline fixed64 fixed_min64(fixed64 a,fixed64 b)
{
    return min_int64(a,b);
}

#endif /*__AOS_KERNEL_SUPPORT_FIXED_H__*/
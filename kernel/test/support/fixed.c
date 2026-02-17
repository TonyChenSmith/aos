/**
 * 内核定点数库函数测试。
 * @date 2026-02-12
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/utest.h>

#include <support/fixed.h>

/**
 * 测试32位定点数打包函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_pack32_basic)
{
    fixed32 fixed=pack_fixed32(1,0x8000);
    UTEST_ASSERT_EQUAL(fixed,0x00018000);
    
    fixed=pack_fixed32(-1,0x8000);
    UTEST_ASSERT_EQUAL(fixed,0xFFFF8000);
    
    fixed=pack_fixed32(0,0x0000);
    UTEST_ASSERT_EQUAL(fixed,0x00000000);
    
    fixed=pack_fixed32(10,0xC000);
    UTEST_ASSERT_EQUAL(fixed,0x000AC000);
    
    UTEST_ASSERT_EQUAL(pack_fixed32(2,0x4000),0x00024000);
    UTEST_ASSERT_EQUAL(pack_fixed32(-3,0xA000),0xFFFDA000);
    UTEST_ASSERT_EQUAL(pack_fixed32(100,0x2000),0x00642000);
    UTEST_ASSERT_EQUAL(pack_fixed32(-50,0xE000),0xFFCEE000);
}

/**
 * 测试64位定点数打包函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_pack64_basic)
{
    fixed64 fixed=pack_fixed64(1,0x80000000);
    UTEST_ASSERT_EQUAL(fixed,0x0000000180000000);
    
    fixed=pack_fixed64(-1,0x80000000);
    UTEST_ASSERT_EQUAL(fixed,0xFFFFFFFF80000000);
    
    fixed=pack_fixed64(0,0x00000000);
    UTEST_ASSERT_EQUAL(fixed,0x0000000000000000);
    
    fixed=pack_fixed64(100,0x40000000);
    UTEST_ASSERT_EQUAL(fixed,0x0000006440000000);
    
    UTEST_ASSERT_EQUAL(pack_fixed64(5,0xC0000000),0x00000005C0000000);
    UTEST_ASSERT_EQUAL(pack_fixed64(-2,0x20000000),0xFFFFFFFE20000000);
    UTEST_ASSERT_EQUAL(pack_fixed64(1000,0x10000000),0x000003E810000000);
    UTEST_ASSERT_EQUAL(pack_fixed64(-500,0xF0000000),0xFFFFFE0CF0000000);
}

/**
 * 测试32位定点数创建函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_make32_basic)
{
    fixed32 fixed=make_fixed32(false,1,0x8000);
    UTEST_ASSERT_EQUAL(fixed,0x00018000);
    
    fixed=make_fixed32(true,0,0x8000);
    UTEST_ASSERT_EQUAL(fixed,0xFFFF8000);
    
    fixed=make_fixed32(false,0,0x0000);
    UTEST_ASSERT_EQUAL(fixed,0x00000000);
    
    fixed=make_fixed32(false,10,0xC000);
    UTEST_ASSERT_EQUAL(fixed,0x000AC000);
    
    UTEST_ASSERT_EQUAL(make_fixed32(false,2,0x4000),0x00024000);
    UTEST_ASSERT_EQUAL(make_fixed32(true,2,0x6000),0xFFFDA000);
    UTEST_ASSERT_EQUAL(make_fixed32(false,100,0x2000),0x00642000);
    UTEST_ASSERT_EQUAL(make_fixed32(true,49,0x2000),0xFFCEE000);
}

/**
 * 测试64位定点数创建函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_make64_basic)
{
    fixed64 fixed=make_fixed64(false,1,0x80000000);
    UTEST_ASSERT_EQUAL(fixed,0x0000000180000000);
    
    fixed=make_fixed64(true,0,0x80000000);
    UTEST_ASSERT_EQUAL(fixed,0xFFFFFFFF80000000);
    
    fixed=make_fixed64(false,0,0x00000000);
    UTEST_ASSERT_EQUAL(fixed,0x0000000000000000);
    
    fixed=make_fixed64(false,100,0x40000000);
    UTEST_ASSERT_EQUAL(fixed,0x0000006440000000);
    
    UTEST_ASSERT_EQUAL(make_fixed64(false,5,0xC0000000),0x00000005C0000000);
    UTEST_ASSERT_EQUAL(make_fixed64(true,1,0xE0000000),0xFFFFFFFE20000000);
    UTEST_ASSERT_EQUAL(make_fixed64(false,1000,0x10000000),0x000003E810000000);
    UTEST_ASSERT_EQUAL(make_fixed64(true,499,0x10000000),0xFFFFFE0CF0000000);
}

/**
 * 测试32位定点数扩展为64位定点数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_expand32_basic)
{
    fixed32 fixed32_val=pack_fixed32(1,0x8000);
    fixed64 expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,0x0000000180000000);
    
    fixed32_val=pack_fixed32(-1,0x8000);
    expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,0xFFFFFFFF80000000);
    
    fixed32_val=pack_fixed32(0,0x0000);
    expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,0x0000000000000000);
    
    fixed32_val=pack_fixed32(100,0x4000);
    expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,0x0000006440000000);
    
    expanded=expand_fixed32(pack_fixed32(2,0x2000));
    UTEST_ASSERT_EQUAL(expanded,0x0000000220000000);
    
    expanded=expand_fixed32(pack_fixed32(-3,0xC000));
    UTEST_ASSERT_EQUAL(expanded,0xFFFFFFFDC0000000);
}

/**
 * 测试64位定点数四舍五入为32位定点数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_round64_basic)
{
    fixed64 fixed=pack_fixed64(1,0x80000000);
    fixed32 rounded=round_fixed64(fixed);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(1,0x8000));
    
    fixed=pack_fixed64(1,0x7FFFFFFF);
    rounded=round_fixed64(fixed);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(1,0x8000));
    
    fixed=pack_fixed64(1,0x00008000);
    rounded=round_fixed64(fixed);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(1,0x0001));
    
    fixed=pack_fixed64(-1,0x80000000);
    rounded=round_fixed64(fixed);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(-1,0x8000));
    
    fixed=pack_fixed64(-1,0x7FFFFFFF);
    rounded=round_fixed64(fixed);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(-1,0x8000));
    
    UTEST_ASSERT_EQUAL(round_fixed64(pack_fixed64(5,0xC0000000)),
        pack_fixed32(5,0xC000));
    UTEST_ASSERT_EQUAL(round_fixed64(pack_fixed64(5,0x3FFFFFFF)),
        pack_fixed32(5,0x4000));
    UTEST_ASSERT_EQUAL(round_fixed64(pack_fixed64(-2,0x40000000)),
        pack_fixed32(-2,0x4000));
    UTEST_ASSERT_EQUAL(round_fixed64(pack_fixed64(-2,0xBFFFFFFF)),
        pack_fixed32(-2,0xC000));
}

/**
 * 测试32位定点数获取整数字面值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_integer32_basic)
{
    fixed32 fixed=pack_fixed32(1,0x8000);
    int16 integer=fixed_integer32(fixed);
    UTEST_ASSERT_EQUAL(integer,1);
    
    fixed=pack_fixed32(-1,0x8000);
    integer=fixed_integer32(fixed);
    UTEST_ASSERT_EQUAL(integer,0);
    
    fixed=pack_fixed32(0,0x0000);
    integer=fixed_integer32(fixed);
    UTEST_ASSERT_EQUAL(integer,0);
    
    fixed=pack_fixed32(100,0x4000);
    integer=fixed_integer32(fixed);
    UTEST_ASSERT_EQUAL(integer,100);
    
    UTEST_ASSERT_EQUAL(fixed_integer32(pack_fixed32(2,0x2000)),2);
    UTEST_ASSERT_EQUAL(fixed_integer32(pack_fixed32(-3,0xC000)),-2);
    UTEST_ASSERT_EQUAL(fixed_integer32(pack_fixed32(0,0xFFFF)),0);
    UTEST_ASSERT_EQUAL(fixed_integer32(pack_fixed32(-10,0x0001)),-9);
}

/**
 * 测试64位定点数获取整数字面值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_integer64_basic)
{
    fixed64 fixed=pack_fixed64(1,0x80000000);
    int32 integer=fixed_integer64(fixed);
    UTEST_ASSERT_EQUAL(integer,1);
    
    fixed=pack_fixed64(-1,0x80000000);
    integer=fixed_integer64(fixed);
    UTEST_ASSERT_EQUAL(integer,0);
    
    fixed=pack_fixed64(0,0x00000000);
    integer=fixed_integer64(fixed);
    UTEST_ASSERT_EQUAL(integer,0);
    
    fixed=pack_fixed64(1000,0x40000000);
    integer=fixed_integer64(fixed);
    UTEST_ASSERT_EQUAL(integer,1000);
    
    UTEST_ASSERT_EQUAL(fixed_integer64(pack_fixed64(500,0x20000000)),500);
    UTEST_ASSERT_EQUAL(fixed_integer64(pack_fixed64(-200,0xC0000000)),-199);
    UTEST_ASSERT_EQUAL(fixed_integer64(pack_fixed64(0,0xFFFFFFFF)),0);
    UTEST_ASSERT_EQUAL(fixed_integer64(pack_fixed64(-100,0x00000001)),-99);
}

/**
 * 测试32位定点数获取分数字面值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_fraction32_basic)
{
    fixed32 fixed=pack_fixed32(1,0x8000);
    uint16 fraction=fixed_fraction32(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x8000);
    
    fixed=pack_fixed32(-1,0x8000);
    fraction=fixed_fraction32(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x8000);
    
    fixed=pack_fixed32(0,0x0000);
    fraction=fixed_fraction32(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x0000);
    
    fixed=pack_fixed32(10,0x4000);
    fraction=fixed_fraction32(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x4000);
    
    UTEST_ASSERT_EQUAL(fixed_fraction32(pack_fixed32(2,0x2000)),0x2000);
    UTEST_ASSERT_EQUAL(fixed_fraction32(pack_fixed32(-3,0xC000)),0x4000);
    UTEST_ASSERT_EQUAL(fixed_fraction32(pack_fixed32(0,0xFFFF)),0xFFFF);
    UTEST_ASSERT_EQUAL(fixed_fraction32(pack_fixed32(-100,0x1234)),0xEDCC);
}

/**
 * 测试64位定点数获取分数字面值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_fraction64_basic)
{
    fixed64 fixed=pack_fixed64(1,0x80000000);
    uint32 fraction=fixed_fraction64(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x80000000);
    
    fixed=pack_fixed64(-1,0x80000000);
    fraction=fixed_fraction64(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x80000000);
    
    fixed=pack_fixed64(0,0x00000000);
    fraction=fixed_fraction64(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x00000000);
    
    fixed=pack_fixed64(100,0x40000000);
    fraction=fixed_fraction64(fixed);
    UTEST_ASSERT_EQUAL(fraction,0x40000000);
    
    UTEST_ASSERT_EQUAL(fixed_fraction64(pack_fixed64(50,0x20000000)),0x20000000);
    UTEST_ASSERT_EQUAL(fixed_fraction64(pack_fixed64(-25,0xC0000000)),0x40000000);
    UTEST_ASSERT_EQUAL(fixed_fraction64(pack_fixed64(0,0xFFFFFFFF)),0xFFFFFFFF);
    UTEST_ASSERT_EQUAL(fixed_fraction64(pack_fixed64(-75,0x12345678)),0xEDCBA988);
}

/**
 * 测试32位定点数加法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_add32_basic)
{
    fixed32 a=pack_fixed32(1,0x4000);
    fixed32 b=pack_fixed32(2,0x4000);
    fixed32 sum=fixed_add32(a,b);
    UTEST_ASSERT_EQUAL(sum,pack_fixed32(3,0x8000));
    
    a=pack_fixed32(-1,0x8000);
    b=pack_fixed32(2,0x4000);
    sum=fixed_add32(a,b);
    UTEST_ASSERT_EQUAL(sum,pack_fixed32(1,0xC000));
    
    a=pack_fixed32(0,0x0000);
    b=pack_fixed32(0,0x0000);
    sum=fixed_add32(a,b);
    UTEST_ASSERT_EQUAL(sum,pack_fixed32(0,0x0000));
    
    UTEST_ASSERT_EQUAL(fixed_add32(pack_fixed32(5,0x2000),pack_fixed32(3,0x3000)),
        pack_fixed32(8,0x5000));
    UTEST_ASSERT_EQUAL(fixed_add32(pack_fixed32(-2,0x8000),pack_fixed32(1,0x8000)),
        pack_fixed32(0,0x0000));
    UTEST_ASSERT_EQUAL(fixed_add32(pack_fixed32(100,0x0000),
        pack_fixed32(-50,0x0000)),pack_fixed32(50,0x0000));
    UTEST_ASSERT_EQUAL(fixed_add32(pack_fixed32(0,0xFFFF),pack_fixed32(0,0x0001)),
        pack_fixed32(1,0x0000));
}

/**
 * 测试64位定点数加法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_add64_basic)
{
    fixed64 a=pack_fixed64(1,0x40000000);
    fixed64 b=pack_fixed64(2,0x40000000);
    fixed64 sum=fixed_add64(a,b);
    UTEST_ASSERT_EQUAL(sum,pack_fixed64(3,0x80000000));
    
    a=pack_fixed64(-1,0x80000000);
    b=pack_fixed64(2,0x40000000);
    sum=fixed_add64(a,b);
    UTEST_ASSERT_EQUAL(sum,pack_fixed64(1,0xC0000000));
    
    a=pack_fixed64(0,0x00000000);
    b=pack_fixed64(0,0x00000000);
    sum=fixed_add64(a,b);
    UTEST_ASSERT_EQUAL(sum,pack_fixed64(0,0x00000000));
    
    UTEST_ASSERT_EQUAL(fixed_add64(pack_fixed64(500,0x20000000),
        pack_fixed64(300,0x30000000)),pack_fixed64(800,0x50000000));
    UTEST_ASSERT_EQUAL(fixed_add64(pack_fixed64(-200,0x80000000),
        pack_fixed64(100,0x80000000)),pack_fixed64(-99,0x00000000));
    UTEST_ASSERT_EQUAL(fixed_add64(pack_fixed64(1000,0x00000000),
        pack_fixed64(-500,0x00000000)),pack_fixed64(500,0x00000000));
    UTEST_ASSERT_EQUAL(fixed_add64(pack_fixed64(0,0xFFFFFFFF),
        pack_fixed64(0,0x00000001)),pack_fixed64(1,0x00000000));
}

/**
 * 测试32位定点数减法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_sub32_basic)
{
    fixed32 a=pack_fixed32(5,0x4000);
    fixed32 b=pack_fixed32(2,0x2000);
    fixed32 diff=fixed_sub32(a,b);
    UTEST_ASSERT_EQUAL(diff,pack_fixed32(3,0x2000));
    
    a=pack_fixed32(1,0x8000);
    b=pack_fixed32(2,0x8000);
    diff=fixed_sub32(a,b);
    UTEST_ASSERT_EQUAL(diff,pack_fixed32(-1,0x0000));
    
    a=pack_fixed32(0,0x0000);
    b=pack_fixed32(0,0x0000);
    diff=fixed_sub32(a,b);
    UTEST_ASSERT_EQUAL(diff,pack_fixed32(0,0x0000));
    
    UTEST_ASSERT_EQUAL(fixed_sub32(pack_fixed32(10,0x5000),pack_fixed32(3,0x2000)),
        pack_fixed32(7,0x3000));
    UTEST_ASSERT_EQUAL(fixed_sub32(pack_fixed32(-5,0x8000),pack_fixed32(-2,0x4000)),
        pack_fixed32(-3,0x4000));
    UTEST_ASSERT_EQUAL(fixed_sub32(pack_fixed32(100,0x0000),pack_fixed32(50,0x0000)),
        pack_fixed32(50,0x0000));
    UTEST_ASSERT_EQUAL(fixed_sub32(pack_fixed32(0,0x0000),pack_fixed32(1,0x8000)),
        pack_fixed32(-2,0x8000));
}

/**
 * 测试64位定点数减法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_sub64_basic)
{
    fixed64 a=pack_fixed64(5,0x40000000);
    fixed64 b=pack_fixed64(2,0x20000000);
    fixed64 diff=fixed_sub64(a,b);
    UTEST_ASSERT_EQUAL(diff,pack_fixed64(3,0x20000000));
    
    a=pack_fixed64(1,0x80000000);
    b=pack_fixed64(2,0x80000000);
    diff=fixed_sub64(a,b);
    UTEST_ASSERT_EQUAL(diff,pack_fixed64(-1,0x00000000));
    
    a=pack_fixed64(0,0x00000000);
    b=pack_fixed64(0,0x00000000);
    diff=fixed_sub64(a,b);
    UTEST_ASSERT_EQUAL(diff,pack_fixed64(0,0x00000000));
    
    UTEST_ASSERT_EQUAL(fixed_sub64(pack_fixed64(1000,0x50000000),
        pack_fixed64(300,0x20000000)),pack_fixed64(700,0x30000000));
    UTEST_ASSERT_EQUAL(fixed_sub64(pack_fixed64(-500,0x80000000),
        pack_fixed64(-200,0x40000000)),pack_fixed64(-300,0x40000000));
    UTEST_ASSERT_EQUAL(fixed_sub64(pack_fixed64(5000,0x00000000),
        pack_fixed64(2500,0x00000000)),pack_fixed64(2500,0x00000000));
    UTEST_ASSERT_EQUAL(fixed_sub64(pack_fixed64(0,0x00000000),
        pack_fixed64(1,0x80000000)),pack_fixed64(-2,0x80000000));
}

/**
 * 测试32位定点数乘法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_mul32_basic)
{
    fixed32 a=pack_fixed32(2,0x0000);
    fixed32 b=pack_fixed32(3,0x0000);
    fixed32 product=fixed_mul32(a,b);
    UTEST_ASSERT_EQUAL(product,pack_fixed32(6,0x0000));
    
    a=pack_fixed32(1,0x8000);
    b=pack_fixed32(2,0x0000);
    product=fixed_mul32(a,b);
    UTEST_ASSERT_EQUAL(product,pack_fixed32(3,0x0000));
    
    a=pack_fixed32(-1,0x8000);
    b=pack_fixed32(2,0x0000);
    product=fixed_mul32(a,b);
    UTEST_ASSERT_EQUAL(product,pack_fixed32(-1,0x0000));
    
    UTEST_ASSERT_EQUAL(fixed_mul32(pack_fixed32(4,0x4000),pack_fixed32(2,0x0000)),
        pack_fixed32(8,0x8000));
    UTEST_ASSERT_EQUAL(fixed_mul32(pack_fixed32(-3,0x2000),pack_fixed32(2,0x0000)),
        pack_fixed32(-6,0x4000));
    UTEST_ASSERT_EQUAL(fixed_mul32(pack_fixed32(0,0x8000),pack_fixed32(5,0x0000)),
        pack_fixed32(2,0x8000));
    UTEST_ASSERT_EQUAL(fixed_mul32(pack_fixed32(10,0x0000),pack_fixed32(0,0x8000)),
        pack_fixed32(5,0x0000));
}

/**
 * 测试64位定点数乘法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_mul64_basic)
{
    fixed64 a=pack_fixed64(2,0x00000000);
    fixed64 b=pack_fixed64(3,0x00000000);
    fixed64 product=fixed_mul64(a,b);
    UTEST_ASSERT_EQUAL(product,pack_fixed64(6,0x00000000));
    
    a=pack_fixed64(1,0x80000000);
    b=pack_fixed64(2,0x00000000);
    product=fixed_mul64(a,b);
    UTEST_ASSERT_EQUAL(product,pack_fixed64(3,0x00000000));
    
    a=pack_fixed64(-1,0x80000000);
    b=pack_fixed64(2,0x00000000);
    product=fixed_mul64(a,b);
    UTEST_ASSERT_EQUAL(product,pack_fixed64(-1,0x00000000));
    
    UTEST_ASSERT_EQUAL(fixed_mul64(pack_fixed64(4,0x40000000),
        pack_fixed64(2,0x00000000)),pack_fixed64(8,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_mul64(pack_fixed64(-3,0x20000000),
        pack_fixed64(2,0x00000000)),pack_fixed64(-6,0x40000000));
    UTEST_ASSERT_EQUAL(fixed_mul64(pack_fixed64(0,0x80000000),
        pack_fixed64(5,0x00000000)),pack_fixed64(2,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_mul64(pack_fixed64(10,0x00000000),
        pack_fixed64(0,0x80000000)),pack_fixed64(5,0x00000000));
}

/**
 * 测试32位定点数除法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_div32_basic)
{
    fixed32 a=pack_fixed32(6,0x0000);
    fixed32 b=pack_fixed32(2,0x0000);
    fixed32 quotient=fixed_div32(a,b);
    UTEST_ASSERT_EQUAL(quotient,pack_fixed32(3,0x0000));
    
    a=pack_fixed32(3,0x0000);
    b=pack_fixed32(2,0x0000);
    quotient=fixed_div32(a,b);
    UTEST_ASSERT_EQUAL(quotient,pack_fixed32(1,0x8000));
    
    a=pack_fixed32(-3,0x0000);
    b=pack_fixed32(2,0x0000);
    quotient=fixed_div32(a,b);
    UTEST_ASSERT_EQUAL(quotient,pack_fixed32(-2,0x8000));
    
    UTEST_ASSERT_EQUAL(fixed_div32(pack_fixed32(8,0x8000),pack_fixed32(2,0x0000)),
        pack_fixed32(4,0x4000));
    UTEST_ASSERT_EQUAL(fixed_div32(pack_fixed32(-6,0x4000),pack_fixed32(2,0x0000)),
        pack_fixed32(-3,0x2000));
    UTEST_ASSERT_EQUAL(fixed_div32(pack_fixed32(2,0x8000),pack_fixed32(5,0x0000)),
        pack_fixed32(0,0x8000));
    UTEST_ASSERT_EQUAL(fixed_div32(pack_fixed32(5,0x0000),pack_fixed32(10,0x0000)),
        pack_fixed32(0,0x8000));
}

/**
 * 测试64位定点数除法。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_div64_basic)
{
    fixed64 a=pack_fixed64(6,0x00000000);
    fixed64 b=pack_fixed64(2,0x00000000);
    fixed64 quotient=fixed_div64(a,b);
    UTEST_ASSERT_EQUAL(quotient,pack_fixed64(3,0x00000000));
    
    a=pack_fixed64(3,0x00000000);
    b=pack_fixed64(2,0x00000000);
    quotient=fixed_div64(a,b);
    UTEST_ASSERT_EQUAL(quotient,pack_fixed64(1,0x80000000));
    
    a=pack_fixed64(-3,0x00000000);
    b=pack_fixed64(2,0x00000000);
    quotient=fixed_div64(a,b);
    UTEST_ASSERT_EQUAL(quotient,pack_fixed64(-2,0x80000000));
    
    UTEST_ASSERT_EQUAL(fixed_div64(pack_fixed64(8,0x80000000),
        pack_fixed64(2,0x00000000)),pack_fixed64(4,0x40000000));
    UTEST_ASSERT_EQUAL(fixed_div64(pack_fixed64(-6,0x40000000),
        pack_fixed64(2,0x00000000)),pack_fixed64(-3,0x20000000));
    UTEST_ASSERT_EQUAL(fixed_div64(pack_fixed64(2,0x80000000),
        pack_fixed64(5,0x00000000)),pack_fixed64(0,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_div64(pack_fixed64(5,0x00000000),
        pack_fixed64(10,0x00000000)),pack_fixed64(0,0x80000000));
}

/**
 * 测试32位定点数绝对值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_abs32_basic)
{
    fixed32 fixed=pack_fixed32(5,0x4000);
    fixed32 abs_val=fixed_abs32(fixed);
    UTEST_ASSERT_EQUAL(abs_val,pack_fixed32(5,0x4000));
    
    fixed=pack_fixed32(-3,0x8000);
    abs_val=fixed_abs32(fixed);
    UTEST_ASSERT_EQUAL(abs_val,pack_fixed32(2,0x8000));
    
    fixed=pack_fixed32(0,0x0000);
    abs_val=fixed_abs32(fixed);
    UTEST_ASSERT_EQUAL(abs_val,pack_fixed32(0,0x0000));
    
    UTEST_ASSERT_EQUAL(fixed_abs32(pack_fixed32(-10,0x2000)),
        pack_fixed32(9,0xE000));
    UTEST_ASSERT_EQUAL(fixed_abs32(pack_fixed32(100,0xC000)),
        pack_fixed32(100,0xC000));
    UTEST_ASSERT_EQUAL(fixed_abs32(pack_fixed32(0,0x8000)),
        pack_fixed32(0,0x8000));
    UTEST_ASSERT_EQUAL(fixed_abs32(pack_fixed32(-0x7FFF,0x0000)),
        pack_fixed32(0x7FFF,0x0000));
}

/**
 * 测试64位定点数绝对值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_abs64_basic)
{
    fixed64 fixed=pack_fixed64(5,0x40000000);
    fixed64 abs_val=fixed_abs64(fixed);
    UTEST_ASSERT_EQUAL(abs_val,pack_fixed64(5,0x40000000));
    
    fixed=pack_fixed64(-3,0x80000000);
    abs_val=fixed_abs64(fixed);
    UTEST_ASSERT_EQUAL(abs_val,pack_fixed64(2,0x80000000));
    
    fixed=pack_fixed64(0,0x00000000);
    abs_val=fixed_abs64(fixed);
    UTEST_ASSERT_EQUAL(abs_val,pack_fixed64(0,0x00000000));
    
    UTEST_ASSERT_EQUAL(fixed_abs64(pack_fixed64(-1000,0x20000000)),
        pack_fixed64(999,0xE0000000));
    UTEST_ASSERT_EQUAL(fixed_abs64(pack_fixed64(5000,0xC0000000)),
        pack_fixed64(5000,0xC0000000));
    UTEST_ASSERT_EQUAL(fixed_abs64(pack_fixed64(0,0x80000000)),
        pack_fixed64(0,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_abs64(pack_fixed64(-0x7FFFFFFF,0x00000000)),
        pack_fixed64(0x7FFFFFFF,0x00000000));
}

/**
 * 测试32位定点数最大值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_max32_basic)
{
    fixed32 a=pack_fixed32(5,0x4000);
    fixed32 b=pack_fixed32(3,0x8000);
    fixed32 max_val=fixed_max32(a,b);
    UTEST_ASSERT_EQUAL(max_val,a);
    
    a=pack_fixed32(-2,0x0000);
    b=pack_fixed32(-1,0x8000);
    max_val=fixed_max32(a,b);
    UTEST_ASSERT_EQUAL(max_val,b);
    
    a=pack_fixed32(0,0x0000);
    b=pack_fixed32(0,0x0000);
    max_val=fixed_max32(a,b);
    UTEST_ASSERT_EQUAL(max_val,a);
    
    UTEST_ASSERT_EQUAL(fixed_max32(pack_fixed32(10,0x0000),pack_fixed32(10,0x0000)),
        pack_fixed32(10,0x0000));
    UTEST_ASSERT_EQUAL(fixed_max32(pack_fixed32(-5,0x8000),pack_fixed32(-3,0x4000)),
        pack_fixed32(-3,0x4000));
    UTEST_ASSERT_EQUAL(fixed_max32(pack_fixed32(100,0xC000),
        pack_fixed32(100,0x8000)),pack_fixed32(100,0xC000));
    UTEST_ASSERT_EQUAL(fixed_max32(pack_fixed32(0,0xFFFF),pack_fixed32(0,0xFFFE)),
        pack_fixed32(0,0xFFFF));
}

/**
 * 测试64位定点数最大值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_max64_basic)
{
    fixed64 a=pack_fixed64(5,0x40000000);
    fixed64 b=pack_fixed64(3,0x80000000);
    fixed64 max_val=fixed_max64(a,b);
    UTEST_ASSERT_EQUAL(max_val,a);
    
    a=pack_fixed64(-2,0x00000000);
    b=pack_fixed64(-1,0x80000000);
    max_val=fixed_max64(a,b);
    UTEST_ASSERT_EQUAL(max_val,b);
    
    a=pack_fixed64(0,0x00000000);
    b=pack_fixed64(0,0x00000000);
    max_val=fixed_max64(a,b);
    UTEST_ASSERT_EQUAL(max_val,a);
    
    UTEST_ASSERT_EQUAL(fixed_max64(pack_fixed64(1000,0x00000000),
        pack_fixed64(1000,0x00000000)),pack_fixed64(1000,0x00000000));
    UTEST_ASSERT_EQUAL(fixed_max64(pack_fixed64(-500,0x80000000),
        pack_fixed64(-300,0x40000000)),pack_fixed64(-300,0x40000000));
    UTEST_ASSERT_EQUAL(fixed_max64(pack_fixed64(5000,0xC0000000),
        pack_fixed64(5000,0x80000000)),pack_fixed64(5000,0xC0000000));
    UTEST_ASSERT_EQUAL(fixed_max64(pack_fixed64(0,0xFFFFFFFF),
        pack_fixed64(0,0xFFFFFFFE)),pack_fixed64(0,0xFFFFFFFF));
}

/**
 * 测试32位定点数最小值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_min32_basic)
{
    fixed32 a=pack_fixed32(5,0x4000);
    fixed32 b=pack_fixed32(3,0x8000);
    fixed32 min_val=fixed_min32(a,b);
    UTEST_ASSERT_EQUAL(min_val,b);
    
    a=pack_fixed32(-2,0x0000);
    b=pack_fixed32(-1,0x8000);
    min_val=fixed_min32(a,b);
    UTEST_ASSERT_EQUAL(min_val,a);
    
    a=pack_fixed32(0,0x0000);
    b=pack_fixed32(0,0x0000);
    min_val=fixed_min32(a,b);
    UTEST_ASSERT_EQUAL(min_val,a);
    
    UTEST_ASSERT_EQUAL(fixed_min32(pack_fixed32(10,0x0000),pack_fixed32(10,0x0000)),
        pack_fixed32(10,0x0000));
    UTEST_ASSERT_EQUAL(fixed_min32(pack_fixed32(-5,0x8000),pack_fixed32(-3,0x4000)),
            pack_fixed32(-5,0x8000));
    UTEST_ASSERT_EQUAL(fixed_min32(pack_fixed32(100,0xC000),
        pack_fixed32(100,0x8000)),pack_fixed32(100,0x8000));
    UTEST_ASSERT_EQUAL(fixed_min32(pack_fixed32(0,0xFFFF),pack_fixed32(0,0xFFFE)),
        pack_fixed32(0,0xFFFE));
}

/**
 * 测试64位定点数最小值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_min64_basic)
{
    fixed64 a=pack_fixed64(5,0x40000000);
    fixed64 b=pack_fixed64(3,0x80000000);
    fixed64 min_val=fixed_min64(a,b);
    UTEST_ASSERT_EQUAL(min_val,b);
    
    a=pack_fixed64(-2,0x00000000);
    b=pack_fixed64(-1,0x80000000);
    min_val=fixed_min64(a,b);
    UTEST_ASSERT_EQUAL(min_val,a);
    
    a=pack_fixed64(0,0x00000000);
    b=pack_fixed64(0,0x00000000);
    min_val=fixed_min64(a,b);
    UTEST_ASSERT_EQUAL(min_val,a);
    
    UTEST_ASSERT_EQUAL(fixed_min64(pack_fixed64(1000,0x00000000),
        pack_fixed64(1000,0x00000000)),pack_fixed64(1000,0x00000000));
    UTEST_ASSERT_EQUAL(fixed_min64(pack_fixed64(-500,0x80000000),
        pack_fixed64(-300,0x40000000)),pack_fixed64(-500,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_min64(pack_fixed64(5000,0xC0000000),
        pack_fixed64(5000,0x80000000)),pack_fixed64(5000,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_min64(pack_fixed64(0,0xFFFFFFFF),
        pack_fixed64(0,0xFFFFFFFE)),pack_fixed64(0,0xFFFFFFFE));
}

/**
 * 测试边界情况和特殊值。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_edge_cases)
{
    UTEST_ASSERT_EQUAL(pack_fixed32(0x7FFF,0xFFFF),0x7FFFFFFF);
    UTEST_ASSERT_EQUAL(pack_fixed32(0x8000,0x0000),0x80000000);
    UTEST_ASSERT_EQUAL(fixed_integer32(0x7FFFFFFF),0x7FFF);
    UTEST_ASSERT_EQUAL(fixed_integer32(0x80000000),(int16)0x8001);
    
    UTEST_ASSERT_EQUAL(pack_fixed64(0x7FFFFFFF,0xFFFFFFFF),0x7FFFFFFFFFFFFFFF);
    UTEST_ASSERT_EQUAL(pack_fixed64(0x80000000,0x00000000),0x8000000000000000);
    UTEST_ASSERT_EQUAL(fixed_integer64(0x7FFFFFFFFFFFFFFF),0x7FFFFFFF);
    UTEST_ASSERT_EQUAL(fixed_integer64(0x8000000000000000),0x80000001);
    
    UTEST_ASSERT_EQUAL(pack_fixed32(0,0x0000),0);
    UTEST_ASSERT_EQUAL(pack_fixed64(0,0x00000000),0);
    UTEST_ASSERT_EQUAL(fixed_integer32(0),0);
    UTEST_ASSERT_EQUAL(fixed_integer64(0),0);
    UTEST_ASSERT_EQUAL(fixed_fraction32(0),0);
    UTEST_ASSERT_EQUAL(fixed_fraction64(0),0);
    
    UTEST_ASSERT_EQUAL(fixed_abs32(-0x80000000),-0x80000000);
    UTEST_ASSERT_EQUAL(fixed_abs64(-0x8000000000000000),-0x8000000000000000);
    
    UTEST_ASSERT_EQUAL(fixed_abs32(pack_fixed32(-1,0x0000)),
        pack_fixed32(1,0x0000));
    UTEST_ASSERT_EQUAL(fixed_abs64(pack_fixed64(-1,0x00000000)),
        pack_fixed64(1,0x00000000));
}

/**
 * 测试定点数运算的完整性。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_comprehensive_operations)
{
    fixed32 a32=pack_fixed32(10,0x4000);
    fixed32 b32=pack_fixed32(3,0x8000);
    fixed32 c32=pack_fixed32(2,0x0000);
    
    fixed32 result32=fixed_mul32(fixed_add32(a32,b32),c32);
    UTEST_ASSERT_EQUAL(result32,pack_fixed32(27,0x8000));
    
    result32=fixed_div32(fixed_sub32(a32,b32),c32);
    UTEST_ASSERT_EQUAL(result32,pack_fixed32(3,0x6000));
    
    fixed64 a64=pack_fixed64(1000,0x40000000);
    fixed64 b64=pack_fixed64(300,0x80000000);
    fixed64 c64=pack_fixed64(2,0x00000000);
    
    fixed64 result64=fixed_mul64(fixed_add64(a64,b64),c64);
    UTEST_ASSERT_EQUAL(result64,pack_fixed64(2601,0x80000000));
    
    result64=fixed_div64(fixed_sub64(a64,b64),c64);
    UTEST_ASSERT_EQUAL(result64,pack_fixed64(349,0xE0000000));
    
    UTEST_ASSERT_EQUAL(fixed_max32(a32,b32),a32);
    UTEST_ASSERT_EQUAL(fixed_min32(a32,b32),b32);
    UTEST_ASSERT_EQUAL(fixed_max64(a64,b64),a64);
    UTEST_ASSERT_EQUAL(fixed_min64(a64,b64),b64);
    
    fixed32 neg32=pack_fixed32(-5,0x8000);
    UTEST_ASSERT_EQUAL(fixed_abs32(neg32),pack_fixed32(4,0x8000));
    UTEST_ASSERT_EQUAL(fixed_integer32(neg32),-4);
    UTEST_ASSERT_EQUAL(fixed_fraction32(neg32),0x8000);
    
    fixed64 neg64=pack_fixed64(-500,0x80000000);
    UTEST_ASSERT_EQUAL(fixed_abs64(neg64),pack_fixed64(499,0x80000000));
    UTEST_ASSERT_EQUAL(fixed_integer64(neg64),-499);
    UTEST_ASSERT_EQUAL(fixed_fraction64(neg64),0x80000000);
}

/**
 * 测试扩展和舍入的完整性。
 * 
 * @return 无返回值。
 */
UTEST_CASE(fixed_expansion_rounding_comprehensive)
{
    fixed32 fixed32_val=pack_fixed32(123,0xABCD);
    fixed64 expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,pack_fixed64(123,0xABCD0000));
    
    fixed64 fixed64_val=pack_fixed64(456,0x12345678);
    fixed32 rounded=round_fixed64(fixed64_val);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(456,0x1234));
    
    fixed32_val=pack_fixed32(789,0x8000);
    expanded=expand_fixed32(fixed32_val);
    rounded=round_fixed64(expanded);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(789,0x8000));
    
    fixed32_val=pack_fixed32(-123,0x8000);
    expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,pack_fixed64(-123,0x80000000));
    
    rounded=round_fixed64(expanded);
    UTEST_ASSERT_EQUAL(rounded,pack_fixed32(-123,0x8000));
    
    fixed32_val=0x7FFFFFFF;
    expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,0x7FFFFFFF0000);
    
    fixed32_val=0x80000000;
    expanded=expand_fixed32(fixed32_val);
    UTEST_ASSERT_EQUAL(expanded,0xFFFF800000000000);
}

/**
 * 定点数库函数测试。
 * 
 * @return 失败测试数。
 */
int32 fixed_test()
{
    UTEST_SUITE("aos.kernel.test.support.fixed");
    
    UTEST_RUN(fixed_pack32_basic);
    UTEST_RUN(fixed_pack64_basic);
    UTEST_RUN(fixed_make32_basic);
    UTEST_RUN(fixed_make64_basic);
    UTEST_RUN(fixed_expand32_basic);
    UTEST_RUN(fixed_round64_basic);
    UTEST_RUN(fixed_integer32_basic);
    UTEST_RUN(fixed_integer64_basic);
    UTEST_RUN(fixed_fraction32_basic);
    UTEST_RUN(fixed_fraction64_basic);
    UTEST_RUN(fixed_add32_basic);
    UTEST_RUN(fixed_add64_basic);
    UTEST_RUN(fixed_sub32_basic);
    UTEST_RUN(fixed_sub64_basic);
    UTEST_RUN(fixed_mul32_basic);
    UTEST_RUN(fixed_mul64_basic);
    UTEST_RUN(fixed_div32_basic);
    UTEST_RUN(fixed_div64_basic);
    UTEST_RUN(fixed_abs32_basic);
    UTEST_RUN(fixed_abs64_basic);
    UTEST_RUN(fixed_max32_basic);
    UTEST_RUN(fixed_max64_basic);
    UTEST_RUN(fixed_min32_basic);
    UTEST_RUN(fixed_min64_basic);
    UTEST_RUN(fixed_edge_cases);
    UTEST_RUN(fixed_comprehensive_operations);
    UTEST_RUN(fixed_expansion_rounding_comprehensive);
    
    UTEST_SUMMARY("aos.kernel.test.support.fixed");
}
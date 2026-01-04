/**
 * 内核单元测试宏组件。
 * @date 2026-01-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_TEST_UTEST_H__
#define __AOS_KERNEL_TEST_UTEST_H__

#include <stdio.h>
#include <string.h>

#include <lib/ktype.h>

/**
 * 测试通过。
 */
static int32 utest_passed=0;

/**
 * 测试失败。
 */
static int32 utest_failed=0;

/**
 * 测试合集。
 */
static int32 utest_total=0;

/**
 * 测试套具。
 */
#define UTEST_SUITE(name) printf("\n\x1b[36m========= %s =========\x1b[0m\n",name)

/**
 * 测试用例。
 */
#define UTEST_CASE(name)\
    static void utest_##name(void);\
    void utest_run_##name(void)\
    {\
        printf("\x1b[33m[TEST]\x1b[0m %s\n",#name);\
        utest_##name();\
    }\
    static void utest_##name(void)

/**
 * 测试断言。
 */
#define UTEST_ASSERT(cond)\
    do\
    {\
        utest_total++;\
        if(cond)\
        {\
            utest_passed++;\
        }\
        else\
        {\
            utest_failed++; \
            printf("\n  \x1b[31mFAILED\x1b[0m: %s:%d: %s\n",__FILE__,__LINE__,#cond); \
        }\
    } while(0)

/**
 * 断言真。
 */
#define UTEST_ASSERT_TRUE(cond) UTEST_ASSERT(cond)

/**
 * 断言假。
 */
#define UTEST_ASSERT_FALSE(cond) UTEST_ASSERT(!(cond))

/**
 * 断言相等。
 */
#define UTEST_ASSERT_EQUAL(a,b) UTEST_ASSERT((a)==(b))

/**
 * 断言不相等。
 */
#define UTEST_ASSERT_NOT_EQUAL(a,b) UTEST_ASSERT((a)!=(b))

/**
 * 断言空。
 */
#define UTEST_ASSERT_NULL(ptr) UTEST_ASSERT((ptr)==NULL)

/**
 * 断言非空。
 */
#define UTEST_ASSERT_NOT_NULL(ptr) UTEST_ASSERT((ptr)!=NULL)

/**
 * 断言字符串相等。
 */
#define UTEST_ASSERT_STRING_EQUAL(a,b) UTEST_ASSERT(strcmp((a),(b))==0)

/**
 * 断言字符串不相等。
 */
#define UTEST_ASSERT_STRING_NOT_EQUAL(a,b) UTEST_ASSERT(strcmp((a),(b))!=0)

/**
 * 测试运行。
 */
#define UTEST_RUN(name) utest_run_##name()

/**
 * 测试结果汇总。
 */
#define UTEST_SUMMARY(name)\
    do\
    {\
        printf("\n\x1b[36m========= %s =========\x1b[0m\n",name);\
        printf("Total:  %d\n",utest_total);\
        printf("\x1b[32mPassed: %d\x1b[0m\n",utest_passed);\
        printf("\x1b[31mFailed: %d\x1b[0m\n",utest_failed);\
        printf("\x1b[36m========= %s =========\x1b[0m\n",name);\
        return utest_failed;\
    } while(0)

#endif /*__AOS_KERNEL_TEST_UTEST_H__*/
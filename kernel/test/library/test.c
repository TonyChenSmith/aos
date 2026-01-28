/**
 * 内核库测试主程序。
 * @date 2026-01-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/library/test.h>

/**
 * 测试字符操作库函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(char_test)
{
    UTEST_ASSERT_EQUAL(char_test(),0);
}

/**
 * 测试转换库函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(convert_test)
{
    UTEST_ASSERT_EQUAL(convert_test(),0);
}

/**
 * 测试内存库函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_test)
{
    UTEST_ASSERT_EQUAL(memory_test(),0);
}

/**
 * 测试字符串操作库函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_test)
{
    UTEST_ASSERT_EQUAL(string_test(),0);
}

/**
 * 主测试。
 * 
 * @return 失败测试数。
 */
int32 main()
{
    UTEST_SUITE("aos.kernel.test.library");
    
    UTEST_RUN(char_test);
    UTEST_RUN(convert_test);
    UTEST_RUN(memory_test);
    UTEST_RUN(string_test);

    UTEST_END("aos.kernel.test.library");
}
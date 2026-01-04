/**
 * 内核库测试主程序。
 * @date 2026-01-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/lib/test.h>

/**
 * 测试内存库函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory)
{
    UTEST_ASSERT_EQUAL(memory_test(),0);
}

/**
 * 主测试。
 * 
 * @return 失败测试数。
 */
int32 main()
{
    UTEST_SUITE("aos.kernel.test.lib");
    
    UTEST_RUN(memory);

    UTEST_SUMMARY("aos.kernel.test.lib");
}
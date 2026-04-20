/**
 * 内核库测试集。
 * @date 2026-01-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_TEST_SUPPORT_TEST_H__
#define __AOS_KERNEL_TEST_SUPPORT_TEST_H__

#include <test/utest.h>

/**
 * 字符库函数测试。
 * 
 * @return 失败测试数。
 */
int32 char_test(void);

/**
 * 转换库函数测试。
 * 
 * @return 失败测试数。
 */
int32 convert_test(void);

/**
 * 定点数库函数测试。
 * 
 * @return 失败测试数。
 */
int32 fixed_test(void);

/**
 * 格式化输出库函数测试。
 * 
 * @return 失败测试数。
 */
int32 format_test(void);

/**
 * 内存库函数测试。
 * 
 * @return 失败测试数。
 */
int32 memory_test(void);

/**
 * 字符串库函数测试。
 * 
 * @return 失败测试数。
 */
int32 string_test(void);

#endif /*__AOS_KERNEL_TEST_SUPPORT_TEST_H__*/
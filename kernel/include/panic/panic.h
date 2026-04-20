/**
 * 内核恐慌处理。
 * @date 2026-04-18
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_PANIC_PANIC_H__
#define __AOS_KERNEL_PANIC_PANIC_H__

#include <support/const.h>

/**
 * 恐慌函数。调用后不再返回。
 * 
 * @param format 适用于支持库格式化规则的格式化字符串。
 * 
 * @return 无法返回。
 */
void noreturn panic(const char8* format,...);

#endif /*__AOS_KERNEL_PANIC_PANIC_H__*/
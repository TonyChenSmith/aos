/**
 * 内核可变参数库函数。
 * @date 2025-12-23
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_LIB_VARARGS_H__
#define __AOS_KERNEL_LIB_VARARGS_H__

/**
 * 可变参数列表。
 */
typedef __builtin_va_list va_list;

/**
 * 可变参数列表初始化。
 */
#define va_start(list,param) __builtin_va_start(list,param)

/**
 * 可变参数列表清理。
 */
#define va_end(list) __builtin_va_end(list)

/**
 * 可变参数获取。
 */
#define va_arg(list,type) __builtin_va_arg(list,type)

/**
 * 可变参数列表复制。
 */
#define va_copy(dest,src) __builtin_va_copy(dest,src)

#endif /*__AOS_KERNEL_LIB_VARARGS_H__*/
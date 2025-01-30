/*
 * 工具模块函数实现。
 * @date 2025-01-17
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_UTIL_IMPL_H__
#define __AOS_BOOT_UTIL_IMPL_H__

#include "basic_type.h"

/*
 * 对MS ABI两参数函数的调用。对函数boot_util_ms_call_2的实现。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * 
 * @return 被调用函数的返回值。
 */
extern uintn boot_ms_call_2(void* restrict func,uintn arg1,uintn arg2);

/*
 * 对MS ABI四参数函数的调用。对函数boot_util_ms_call_4的实现。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * @param arg3 被调用函数的第三个参数。
 * @param arg4 被调用函数的第四个参数。
 * 
 * @return 被调用函数的返回值。
 */
extern uintn boot_ms_call_4(void* restrict func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

/*
 * 内存复制。行为类似于string.h的memcpy函数。对函数boot_util_memcpy的实现。
 *
 * @param s1 目的起始。
 * @param s2 源起始。
 * @param n  复制数目。
 * 
 * @return 目的起始。
 */
extern void* boot_memcpy(void* restrict s1,const void* restrict s2,uintn n);

/*
 * 内存设置。行为类似于string.h的memset函数。对函数boot_util_memset的实现。
 *
 * @param s	起始地址。
 * @param c	写入值。
 * @param n	写入数目。
 * 
 * @return 目的起始。
 */
extern void* boot_memset(void* restrict s,uint32 c,uintn n);

/*
 * 端口读取。对函数boot_util_readport的实现。
 *
 * @param dest	目的起始。
 * @param width	读入位宽。
 * @param port	读入端口。
 * @param n		读取数目。
 *
 * @return 目的起始。
 */
extern void* boot_readport(void* restrict dest,uint8 width,uint16 port,uintn n);

/*
 * 端口写入。对函数boot_util_writeport的实现。
 *
 * @param src	源起始。
 * @param width	写入位宽。
 * @param port	写入端口。
 * @param n		写入数目。
 *
 * @return 源起始。
 */
extern void* boot_writeport(const void* restrict src,uint8 width,uint16 port,uintn n);

#endif /*__AOS_BOOT_UTIL_IMPL_H__*/

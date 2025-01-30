/*
 * 工具模块定义。定义了所有工具函数和函数表结构。
 * @date 2025-01-17
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_UTIL_H__
#define __AOS_BOOT_UTIL_H__

#include "basic_type.h"

/*
 * 工具模块初始化。
 *
 * @return 无返回值。
 */
typedef void (*boot_util_init)(void);

/*
 * 对MS ABI两参数函数的调用。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * 
 * @return 被调用函数的返回值。
 */
typedef uintn (*boot_util_ms_call_2)(void* restrict func,uintn arg1,uintn arg2);

/*
 * 对MS ABI四参数函数的调用。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * @param arg3 被调用函数的第三个参数。
 * @param arg4 被调用函数的第四个参数。
 * 
 * @return 被调用函数的返回值。
 */
typedef uintn (*boot_util_ms_call_4)(void* restrict func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

/*
 * 内存复制。行为类似于string.h的memcpy函数。
 *
 * @param s1 目的起始。
 * @param s2 源起始。
 * @param n  复制数目。
 * 
 * @return 目的起始。
 */
typedef void* (*boot_util_memcpy)(void* restrict s1,const void* restrict s2,uintn n);

/*
 * 内存设置。行为类似于string.h的memset函数。
 *
 * @param s	起始地址。
 * @param c	写入值。
 * @param n	写入数目。
 * 
 * @return 目的起始。
 */
typedef void* (*boot_util_memset)(void* restrict s,uint32 c,uintn n);

/*端口位宽，分别为8位、16位和32位*/
#define PORT_WIDTH_8 0
#define PORT_WIDTH_16 1
#define PORT_WIDTH_32 2

/*
 * 端口读取。
 *
 * @param dest	目的起始。
 * @param width	读入位宽。
 * @param port	读入端口。
 * @param n		读取数目。
 *
 * @return 目的起始。
 */
typedef void* (*boot_util_readport)(void* restrict dest,uint8 width,uint16 port,uintn n);

/*
 * 端口写入。
 *
 * @param src	源起始。
 * @param width	写入位宽。
 * @param port	写入端口。
 * @param n		写入数目。
 *
 * @return 源起始。
 */
typedef void* (*boot_util_writeport)(const void* restrict src,uint8 width,uint16 port,uintn n);

/*
 * 工具模块函数表。
 */
typedef struct _boot_util_function
{
	boot_util_init init;		   /*模块初始化*/
	boot_util_ms_call_2 ms_call_2; /*MS ABI两参数调用*/
	boot_util_ms_call_4 ms_call_4; /*MS ABI四参数调用*/
	boot_util_memcpy memcpy;	   /*内存复制*/
	boot_util_memset memset;	   /*内存设置*/
	boot_util_readport readport;   /*端口读取*/
	boot_util_writeport writeport; /*端口写入*/
} boot_util_function;

#endif /*__AOS_BOOT_UTIL_H__*/
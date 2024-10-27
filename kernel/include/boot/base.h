/*
 * 基础模块总头文件。定义了所有导出函数和入口函数。
 * @date 2024-10-27
 */
#ifndef __AOS_BOOT_BASE_H__
#define __AOS_BOOT_BASE_H__

#include "type.h"

/*
 * 对MS ABI两参数函数的调用。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * 
 * @return 被调用函数的返回值。
 */
typedef uintn (*boot_ms_call_2)(void* func,uintn arg1,uintn arg2);

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
typedef uintn (*boot_ms_call_4)(void* func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

/*
 * 内存复制。
 */
typedef uintn (*boot_memcpy)(void* s1,const void* s2,uintn n);

/*
 * 基础模块函数表。
 */
typedef struct _boot_base_functions
{
	boot_ms_call_2 boot_ms_call_2;
	boot_ms_call_4 boot_ms_call_4;
} boot_base_functions;

#endif
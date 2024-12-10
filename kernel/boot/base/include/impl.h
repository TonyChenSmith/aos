/*
 * 函数实现。
 * @date 2024-10-28
 */
#ifndef __AOS_BOOT_BASE_IMPL_H__
#define __AOS_BOOT_BASE_IMPL_H__

#include "type.h"

/*
 * 对MS ABI两参数函数的调用，对函数指针类型boot_ms_call_2的实现。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * 
 * @return 被调用函数的返回值。
 */
extern uintn base_ms_call_2(void* restrict func,uintn arg1,uintn arg2);

/*
 * 对MS ABI四参数函数的调用，对函数指针类型boot_ms_call_4的实现。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * @param arg3 被调用函数的第三个参数。
 * @param arg4 被调用函数的第四个参数。
 * 
 * @return 被调用函数的返回值。
 */
extern uintn base_ms_call_4(void* restrict func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

/*
 * 内存复制，行为类似于string.h的memcpy函数，对函数指针类型boot_memcpy的实现。
 *
 * @param s1 输出内存。
 * @param s2 输入内存。
 * @param n  复制字节数。
 * 
 * @return 输出内存。
 */
extern void* base_memcpy(void* restrict s1,const void* restrict s2,uintn n);

/*
 * 内存设置，行为类似于string.h的memset函数，对函数指针类型boot_memset的实现。
 *
 * @param str 写入内存。
 * @param c	  写入值。
 * @param n	  写入字节数。
 * 
 * @return 写入内存。
 */
extern void* base_memset(void* restrict str,const uint8 c,uintn n);

#endif /*__AOS_BOOT_BASE_IMPL_H__*/

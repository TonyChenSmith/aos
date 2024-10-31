/*
 * 基础模块函数实现。定义了实现函数名。
 * @date 2024-10-28
 */
#ifndef __AOS_BOOT_BASE_IMPL_H__
#define __AOS_BOOT_BASE_IMPL_H__

#include "type.h"

/*
 * 函数boot_ms_call_2的实现。
 */
extern uintn base_ms_call_2(void* restrict func,uintn arg1,uintn arg2);

/*
 * 函数boot_ms_call_4的实现。
 */
extern uintn base_ms_call_4(void* restrict func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

/*
 * 函数boot_memcpy的实现。
 */
extern void* base_memcpy(void* restrict s1,const void* restrict s2,uintn n);

#endif /*__AOS_BOOT_BASE_IMPL_H__*/

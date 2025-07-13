/* 
 * 模块“aos.uefi”内存池管理声明。
 * 声明了内存池管理的相关函数。
 * @date 2025-06-06
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MEMORY_H__
#define __AOS_UEFI_MEMORY_H__

#include "defines.h"

/* 
 * 初始化内存池管理。
 * 
 * @param bitmap 位图地址。
 * @param meta   TLSF元数据地址。
 * 
 * @return 正常返回成功，异常返回对应错误。
 */
EFI_STATUS EFIAPI uefi_memory_init(OUT UINTN* bitmap,OUT UINTN* meta);

/* 
 * 在位图内分配连续多页。
 * 
 * @param pages 需要分配的连续页数。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI memory_page_alloc(IN UINTN pages);

/* 
 * 在位图内释放页面。
 * 
 * @param ptr 页面基址。
 * 
 * @return 无返回值。不会释放预分配区页面与错误地址。
 */
VOID EFIAPI memory_page_free(IN VOID* ptr);

/* 
 * 在TLSF内存池内申请内存。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI memory_pool_alloc(IN UINTN size);

/* 
 * 向TLSF内存池内释放内存。
 * 
 * @param ptr 块数据基址。
 * 
 * @return 无返回值。对错误指针无反应。
 */
VOID EFIAPI memory_pool_free(IN VOID* ptr);

/* 
 * 在调试模式转储内存池信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI memory_dump_pool_info(VOID);

/* 
 * 内存池功能测试。
 * 
 * @return 无返回值。
 */
VOID EFIAPI memory_function_test(VOID);

/* 
 * 检查数组中每个地址对应的内存类型。不可达区域标记为保留。调用者有义务保证两个数组的空间存在。
 * 
 * @param addrs  内存地址数组。
 * @param length 数组长度。
 * @param types  内存类型数组。
 * 
 * @return 返回调用状态。
 */
EFI_STATUS EFIAPI memory_get_memory_type(IN UINTN* addrs,IN UINTN length,OUT EFI_MEMORY_TYPE* types);

#endif /*__AOS_UEFI_MEMORY_H__*/
/* 
 * 模块“aos.uefi”内存池管理功能。
 * 声明了相关的跨文件使用函数。
 * @date 2025-06-06
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MEM_H__
#define __AOS_UEFI_MEM_H__

#include "params.h"

/* 
 * 使用内存空间构造一个内存池。调用者有义务保证参数的合理性。
 * 
 * @param addr 内存块地址。
 * @param size 内存块大小。
 * 
 * @return 正常返回成功，大小不足最低要求返回错误。
 */
EFI_STATUS EFIAPI create_pool(IN VOID* addr,IN UINTN size);

/* 
 * 初始化内存池管理功能。这里管理的是引导内存池。
 * 
 * @param bpool 引导内存池基址。
 * 
 * @return 正常返回成功，异常返回对应错误。
 */
EFI_STATUS EFIAPI mem_init(OUT UINTN* bpool);

/* 
 * 在内存池内申请一块内存。
 * 
 * @param pool 内存池基址。
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI pool_alloc(IN VOID* pool,IN UINTN size);

/* 
 * 在引导内存池内申请一块内存。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI malloc(IN UINTN size);

/* 
 * 向内存池内释放内存。调用者有义务保证释放指针归属于该内存池，为了性能不做指针大范围检查。
 * 
 * @param pool 内存池基址。
 * @param ptr  内存块基址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI pool_free(IN VOID* pool,IN VOID* ptr);

/* 
 * 向引导内存池内释放内存。调用者有义务保证释放指针归属于该内存池，为了性能不做指针大范围检查。
 * 
 * @param ptr 内存块基址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI free(IN VOID* ptr);

/* 
 * 在调试模式转储输入的内存池信息。
 * 
 * @param pool 内存池基址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_pool_info(IN VOID* pool);

/* 
 * 检查数组中每个地址对应的内存类型。不可达区域标记为保留。调用者有义务保证两个数组的空间存在。
 * 
 * @param addrs  内存地址数组。
 * @param length 数组长度。
 * @param types  内存类型数组。
 * 
 * @return 返回调用状态。
 */
EFI_STATUS EFIAPI get_memory_type(IN UINTN* addrs,IN UINTN length,OUT EFI_MEMORY_TYPE* types);

#endif /*__AOS_UEFI_MEM_H__*/
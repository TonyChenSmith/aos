/**
 * 模块引导内存池管理。
 * @date 2026-07-19
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_POOL_H__
#define __AOS_UEFI_POOL_H__

#include <Uefi.h>

/**
 * 初始化引导内存池。
 * 
 * @return 申请到的引导内存池基址。
 */
UINTN EFIAPI init_bootstrap_pool(VOID);

/**
 * 在引导内存池内申请一块内存。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI pool_alloc(IN UINTN size);

/**
 * 向引导内存池释放一块内存。
 * 
 * @param pointer 被释放指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI pool_free(IN VOID* pointer);

#endif /*__AOS_UEFI_POOL_H__*/
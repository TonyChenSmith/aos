/**
 * 模块内存标记管理。
 * @date 2026-08-23
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MARK_H__
#define __AOS_UEFI_MARK_H__

#include "params.h"

/**
 * 页面申请并标记。
 * 
 * @param type  申请页面类型。
 * @param pages 申请页数。
 * @param max   申请的最大边界。
 * 
 * @return 申请页面的起始地址。
 */
VOID* EFIAPI allocate_page(IN UINT64 type,IN UINTN pages,IN UINTN max);

/**
 * 用EFI内存图填充AOS内存图。
 * 
 * @param map       EFI内存图。
 * @param map_size  内存图大小。
 * @param desc_size 描述符大小。
 * @param boot_pool 引导内存池基址。
 * 
 * @return 已填充完成的内存图头结点。
 */
aos_boot_memory* EFIAPI fill_memory_map(IN EFI_MEMORY_DESCRIPTOR* map,IN UINTN map_size,IN UINTN desc_size,
    IN UINTN boot_pool);

#endif /*__AOS_UEFI_MARK_H__*/
/**
 * 模块线性区管理。
 * @date 2026-08-09
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_VMA_H__
#define __AOS_UEFI_VMA_H__

#include "params.h"

/**
 * 添加一个线性区。
 * 
 * @param type  线性区类型。
 * @param vaddr 线性区域基址。
 * @param paddr 物理区域基址。
 * @param pages 区域页数。
 * @param flags 线性区标记。
 * 
 * @return 无返回值。
 */
VOID EFIAPI add_vma(IN UINT64 type,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/**
 * 删除一个线性区。仅在通过输入参数能够找到线性区时才会删除。
 * 
 * @param vaddr 线性区域地址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI remove_vma(IN UINTN vaddr);

/**
 * 用EFI内存图和内存属性表填充AOS线性区，并为EFI内存图和启动参数赋值。
 * 
 * @param params    启动参数。
 * @param map       EFI内存图。
 * @param map_size  内存图大小。
 * @param desc_size 描述符大小。
 * 
 * @return 无返回值。
 */
VOID EFIAPI fill_firmware_vma(IN OUT aos_boot_params* params,IN OUT EFI_MEMORY_DESCRIPTOR* map,IN UINTN map_size,
    IN UINTN desc_size);

#endif /*__AOS_UEFI_VMA_H__*/
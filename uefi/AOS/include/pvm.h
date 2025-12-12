/**
 * 模块页表与线性区管理。
 * @date 2025-08-24
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PVM_H__
#define __AOS_UEFI_PVM_H__

#include "params.h"

/**
 * 遍历位图信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_bitmap();

/**
 * 遍历全部内核页表。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_page_table();

/**
 * 添加一个内核线性区。
 * 
 * @param vaddr 线性区域基址。
 * @param paddr 物理区域基址。
 * @param pages 区域页数。
 * @param flags 线性区标记。
 * 
 * @return 正常返回成功。出现问题返回对应错误。
 */
EFI_STATUS EFIAPI add_kernel_vma(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/**
 * 删除一个内核线性区。仅在通过输入参数能够找到线性区时才会删除。
 * 
 * @param vaddr 线性区域地址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI remove_kernel_vma(IN UINTN vaddr);

/**
 * 遍历所有线性区信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_vma();

/**
 * 初始化页表与线性区管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 应该返回成功。
 */
EFI_STATUS EFIAPI pvm_init(IN OUT aos_boot_params* params);

#endif /*__AOS_UEFI_PVM_H__*/
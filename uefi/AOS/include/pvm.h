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
 * 检查一个线性区域是否与存在的已有线性区重叠。
 * 
 * @param vaddr 线性区域地址。
 * @param pages 线性区域页数。从实用角度，调用者有必要输入非零值。
 * 
 * @return 有重叠区域返回真。
 */
BOOLEAN EFIAPI check_vma_overlap(IN UINTN vaddr,IN UINTN pages);

/**
 * 遍历所有线性区信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_vma();

/**
 * 生成随机32位数。内部增强算法，但由于熵源单一还是容易被撞击。参数通过指针传递，方便后续二次加强计算。
 * 
 * @param a 参数A。
 * @param b 参数B。
 * @param c 参数C。
 * 
 * @return 返回一个随机数。
 */
UINT32 EFIAPI random32(IN OUT UINT32* a,IN OUT UINT32* b,IN OUT UINT32* c);

/**
 * 初始化页表与线性区管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 应该返回成功。
 */
EFI_STATUS EFIAPI pvm_init(IN OUT aos_boot_params* params);

#endif /*__AOS_UEFI_PVM_H__*/
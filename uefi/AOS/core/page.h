/**
 * 模块分页管理。
 * @date 2026-08-08
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PAGE_H__
#define __AOS_UEFI_PAGE_H__

#include "params.h"

/**
 * 初始化页表池和页表池位图。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI init_page_table_pool(IN OUT aos_boot_params* params);

/**
 * 将物理地址范围映射到页表的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 无返回值。
 */
VOID EFIAPI page_table_map(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/**
 * 解除页表内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI page_table_unmap(IN UINTN start,IN UINTN end);

/**
 * 设置代码段寄存器。
 * 
 * @param cs 代码段选择子。
 * 
 * @return 无返回值。
 */
VOID EFIAPI set_cs(UINT16 cs);

/**
 * 设置数据段寄存器。
 * 
 * @param ds 数据段选择子。
 * 
 * @return 无返回值。
 */
VOID EFIAPI set_ds(UINT16 ds);

#endif /*__AOS_UEFI_PAGE_H__*/
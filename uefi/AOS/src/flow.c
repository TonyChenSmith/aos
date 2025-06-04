/*
 * 模块“aos.uefi”入口。实现了在UEFI阶段的执行流程。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * 
 * SPDX-License-Identifier: MIT
 */

#include "flow.h"

/*
 * 模块“aos.uefi”的入口函数。
 * 
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 理论上无返回值，返回值为EFI_SUCCESS。
 */
EFI_STATUS EFIAPI aos_uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table)
{
    return EFI_SUCCESS;
}

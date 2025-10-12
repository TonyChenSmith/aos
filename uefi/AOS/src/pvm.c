/* 
 * 模块“aos.uefi”页表与线性区管理功能。
 * 实现了相关函数，以及便于调试的函数。
 * @date 2025-07-13
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "pvmi.h"

/* 
 * 初始化页表与线性区管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 应该返回成功。
 */
EFI_STATUS EFIAPI pvm_init(IN OUT aos_boot_params* params)
{
    return EFI_SUCCESS;
}
/* 
 * 模块“aos.uefi”页表与线性区管理功能。
 * 声明了相关的跨文件使用函数。
 * @date 2025-08-24
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PVM_H__
#define __AOS_UEFI_PVM_H__

#include "params.h"

/* 
 * 初始化页表与线性区管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 应该返回成功。
 */
EFI_STATUS EFIAPI pvm_init(IN OUT aos_boot_params* params);

#endif /*__AOS_UEFI_PVM_H__*/
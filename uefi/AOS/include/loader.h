/**
 * 模块内核程序文件加载。
 * @date 2025-12-08
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_LOADER_H__
#define __AOS_UEFI_LOADER_H__

#include "mem.h"

/**
 * 将内核文件加载到目标区域。
 * 
 * @param params 启动参数。
 * 
 * @return 正常返回成功。
 */
EFI_STATUS EFIAPI load_kernel(IN OUT aos_boot_params* params);

#endif /*__AOS_UEFI_LOADER_H__*/
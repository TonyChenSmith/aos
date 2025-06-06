/*
 * 模块“aos.uefi”内存管理内部实现。
 * 声明了与内存管理相关的函数。
 * @date 2025-06-06
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MEMORY_H__
#define __AOS_UEFI_MEMORY_H__

#include "defines.h"

/*
 * 模块“aos.uefi”的内存池初始化。
 * 
 * @param params 启动参数。
 * 
 * @return 正常返回EFI_SUCCESS。异常返回对应错误码。然后返回启动参数地址。
 */
EFI_STATUS EFIAPI uefi_memory_init(OUT boot_params* params);

#endif /*__AOS_UEFI_MEMORY_H__*/
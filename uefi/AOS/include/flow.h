/*
 * 模块“aos.uefi”流程控制。声明了在UEFI阶段的流程函数。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_FLOW_H__
#define __AOS_UEFI_FLOW_H__

#include "memory.h"

/*
 * 模块“aos.uefi”的入口函数。
 * 
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 理论上无返回值，返回值为EFI_SUCCESS。
 */
EFI_STATUS EFIAPI aos_uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table);

/*
 * 初始化内存池管理。包括申请内存页，构造位图、元数据和块数据。
 * 
 * @param bitmap 位图地址。
 * @param meta   TLSF元数据地址。
 * 
 * @return 正常返回EFI_SUCCESS。异常返回对应错误码。
 */
EFI_STATUS EFIAPI uefi_memory_init(OUT UINTN* bitmap,OUT UINTN* meta);

#endif /*__AOS_UEFI_FLOW_H__*/
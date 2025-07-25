/* 
 * 模块“aos.uefi”流程控制。
 * 声明了相关的跨文件与跨模块使用函数。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_FLOW_H__
#define __AOS_UEFI_FLOW_H__

#include "env.h"
#include "pmm.h"

/* 
 * 模块“aos.uefi”入口。
 * 
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 理论上不返回。
 */
EFI_STATUS EFIAPI aos_uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table);

#endif /*__AOS_UEFI_FLOW_H__*/
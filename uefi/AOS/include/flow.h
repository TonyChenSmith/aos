/**
 * 模块运行流程管理。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_FLOW_H__
#define __AOS_UEFI_FLOW_H__

#include "env.h"
#include "fsm.h"
#include "loader.h"
#include "mem.h"
#include "pvm.h"

#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

/**
 * 输出系统表信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_system_table();

/**
 * 输出启动参数信息。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_boot_params(IN aos_boot_params* params);

/**
 * 内核蹦床函数，在换栈后进入内核入口函数不再返回。
 * 
 * @param params 启动参数。
 * @param stack  栈底。
 * 
 * @return 不再返回。
 */
typedef VOID NORETURN EFIAPI (*aos_kernel_trampoline)(IN aos_boot_params* params,IN UINTN stack);

/**
 * 模块“aos.uefi”入口。
 * 
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 理论上不返回。
 */
EFI_STATUS EFIAPI aos_uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table);

#endif /*__AOS_UEFI_FLOW_H__*/
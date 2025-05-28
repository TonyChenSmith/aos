/*
 * aos.uefi模块定义。定义了在UEFI阶段的启动流程。
 * @date 2025-01-24
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MODULE_H__
#define __AOS_UEFI_MODULE_H__

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "params.h"
//include <Guid/Acpi.h>
//include <Guid/SmBios.h>
//include <Library/BaseLib.h>
//include <Library/BaseMemoryLib.h>
//include <Library/DevicePathLib.h>
//include <Library/MemoryAllocationLib.h>
//include <Library/PrintLib.h>
//include <Library/UefiLib.h>
//include <Library/UefiApplicationEntryPoint.h>
//include <Protocol/LoadedImage.h>

/*
 * 模块入口函数。
 *
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 无返回值。
 */
EFI_STATUS EFIAPI uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table);

/*
 * 初始化固定内存块所需信息，包括启动核参数和设备路径。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_init_fixed(boot_params** restrict params);

/*
 * 初始化启动核，将启动核模块加载进入内存中。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_load_boot(IN boot_params* restrict params);

/*
 * 初始化设备图像信息。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。 
 */
EFI_STATUS EFIAPI uefi_init_graphics(IN OUT boot_params* restrict params);

/*
 * 设置系统表，包括ACPI、SMBIOS和运行时服务。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，总是成功。
 */
EFI_STATUS EFIAPI uefi_set_table(boot_params* restrict params);

/*
 * 获取系统具有核心数与可用核心数。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，总是成功。
 */
EFI_STATUS EFIAPI uefi_get_enable_cores(boot_params* restrict params);

/*
 * 设置PMMS内存块和内存映射内存块。
 *
 * @param params 启动核参数。
 * @param key	 映射键。
 * 
 * @return 状态码，出错返回非零值。
 */
EFI_STATUS EFIAPI uefi_set_memory_map(boot_params* restrict params,OUT UINTN* key);

/*
 * 启动核蹦床函数，进入后正式进入启动核，并且不返回。
 *
 * @param params 启动核参数。
 * 
 * @return 无返回值。
 */
typedef VOID (*aos_boot_trampoline)(boot_params* restrict params);

#endif /*__AOS_UEFI_MODULE_H__*/
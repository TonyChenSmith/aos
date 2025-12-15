/**
 * 模块文件系统管理。
 * @date 2025-12-12
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_FSM_H__
#define __AOS_UEFI_FSM_H__

#include "mem.h"

#include <Library/FileHandleLib.h>

/**
 * 初始化文件系统管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 一定成功。
 */
EFI_STATUS EFIAPI fsm_init(IN OUT aos_boot_params* params);

/**
 * 挂载ESP分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI mount();

/**
 * 卸载ESP分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI umount();

/**
 * 打开ESP内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到和其它错误返回空。其它错误会打印调试输出。
 */
EFI_FILE_HANDLE EFIAPI ufopen(IN CHAR16* path,IN UINT64 mode,IN UINT64 attrs);

/**
 * 快速获取文件的大小。调用者有义务保证参数为文件而非目录。
 * 
 * @param file 句柄。
 * 
 * @return 文件大小。出现问题返回最大值。
 */
UINT64 EFIAPI ufsize(IN EFI_FILE_HANDLE file);

#endif /*__AOS_UEFI_FSM_H__*/
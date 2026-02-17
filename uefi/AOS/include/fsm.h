/**
 * 模块文件系统管理。
 * @date 2025-12-12
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_FSM_H__
#define __AOS_UEFI_FSM_H__

#include "mem.h"

#include <Library/FileHandleLib.h>

/**
 * 系统文件结构。
 */
typedef struct _system_file system_file;

/**
 * 初始化文件系统管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 一定成功。
 */
EFI_STATUS EFIAPI fsm_init(IN OUT aos_boot_params* params);

/**
 * 挂载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_mount();

/**
 * 卸载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_umount();

/**
 * 打开EFI系统分区内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到和其它错误返回空。其它错误会打印调试输出。
 */
EFI_FILE_HANDLE EFIAPI esp_fopen(IN CHAR16* path,IN UINT64 mode,IN UINT64 attrs);

/**
 * 快速获取EFI系统分区内文件的大小。调用者有义务保证参数为文件而非目录。
 * 
 * @param file 句柄。
 * 
 * @return 文件大小。出现问题返回最大值。
 */
UINT64 EFIAPI esp_fsize(IN EFI_FILE_HANDLE file);

/**
 * 根据系统分区设备句柄初始化AOS系统分区的文件系统。
 * 
 * @param system 系统分区设备句柄。
 * 
 * @return 无返回值。
 */
VOID EFIAPI system_fs_init(IN EFI_HANDLE system);

/**
 * 挂载AOS系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI system_mount();

/**
 * 卸载AOS系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI system_unmount();

/**
 * 打开AOS内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * @param mode 打开模式。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
system_file* system_fopen(IN CHAR8* path,IN UINT64 mode);

/**
 * 读取系统分区文件数据。
 * 
 * @param file 文件指针
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 system_fread(IN system_file* file,OUT UINT8* dest,IN UINT64 size);

/**
 * 写入系统分区文件数据。
 * 
 * @param file 文件指针
 * @param dest 源数组。
 * @param size 期望写入大小。
 * 
 * @return 实际写入大小。
 */
UINT64 system_fwrite(IN system_file* file,IN UINT8* src,IN UINT64 size);

#endif /*__AOS_UEFI_FSM_H__*/
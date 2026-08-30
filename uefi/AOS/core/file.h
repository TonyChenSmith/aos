/**
 * 模块文件管理。
 * @date 2026-08-10
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_FILE_H__
#define __AOS_UEFI_FILE_H__

#include <Library/FileHandleLib.h>

#include "params.h"

/**
 * AOS系统卷类型。
 */
typedef enum _asv_type
{
    ASV_ESP, /*EFI系统分区。*/
    ASV_GPT, /*硬盘GPT分区。*/
    ASV_MAX  /*最大系统卷类型。*/
} asv_type;

/**
 * AOS系统卷文件结构。
 */
typedef struct _asv_file asv_file;

/**
 * 初始化EFI系统分区读写。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI init_esp_io(IN OUT aos_boot_params* params);

/**
 * 挂载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_mount(VOID);

/**
 * 卸载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_unmount(VOID);

/**
 * 打开EFI系统分区内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到返回空。
 */
EFI_FILE_HANDLE EFIAPI esp_open(IN CHAR16* path,IN UINT64 mode,IN UINT64 attrs);

/**
 * 快速获取EFI系统分区内文件的大小。调用者有义务保证参数为文件而非目录。
 * 
 * @param file 句柄。
 * 
 * @return 文件大小。出现问题返回最大值。
 */
UINT64 EFIAPI esp_get_size(IN EFI_FILE_HANDLE file);

/**
 * 初始化AOS系统卷读写。
 * 
 * @param type   AOS系统卷类型。
 * @param info   设备识别信息。
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI init_asv_io(IN asv_type type,IN VOID* info,IN OUT aos_boot_params* params);

/**
 * 挂载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_mount(VOID);

/**
 * 卸载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_unmount(VOID);

/**
 * 打开AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
asv_file* EFIAPI asv_open(IN CONST CHAR8* path);

/**
 * 关闭AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_close(IN asv_file* file);

/**
 * 读取AOS系统卷文件数据。指针会最后指向未读的第一个字节。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 EFIAPI asv_read(IN asv_file* file,OUT VOID* dest,IN UINT64 size);

/**
 * 读取AOS系统卷文件指定位置起始的数据。指针会最后指向未读的第一个字节。
 * 
 * @param file   文件指针。
 * @param offset 起始偏移。
 * @param dest   目标数组。
 * @param size   期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 EFIAPI asv_read_at(IN asv_file* file,IN UINT64 offset,OUT VOID* dest,IN UINT64 size);

/**
 * 重置AOS系统卷文件指针位置。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_reset(IN asv_file* file);

/**
 * 从AOS系统卷文件当前指针位置跳过N个字节。
 * 
 * @param file 文件指针。
 * @param n    跳过字节数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_skip(IN asv_file* file,IN UINT64 n);

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * 
 * @return 文件大小。
 */
UINT64 EFIAPI asv_get_size(IN asv_file* file);

#endif /*__AOS_UEFI_FILE_H__*/
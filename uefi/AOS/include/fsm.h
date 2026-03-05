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
 * AOS系统卷文件结构。
 */
typedef struct _asv_file asv_file;

/**
 * AOS系统卷类型。
 */
typedef enum _asv_type
{
    ASV_ESP,            /*EFI系统分区。*/
    ASV_HARD_DRIVE_GPT, /*硬盘驱动GDT分区。*/
    ASV_TYPE_MAX        /*AOS系统卷类型最大值。*/
} asv_type;

/**
 * 位置参考。
 */
typedef enum _position_reference
{
    POSITION_START,        /*起始。*/
    POSITION_CURRENT,      /*当前位置。*/
    POSITION_END,          /*结尾。*/
    POSITION_REFERENCE_MAX /*位置参考最大值。*/
} position_reference;

/**
 * 读打开模式。
 */
#define ASV_OPEN_MODE_READ BIT0

/**
 * 写打开模式。
 */
#define ASV_OPEN_MODE_WRITE BIT1

/**
 * 没有则创建打开模式。
 */
#define ASV_OPEN_MODE_CREATE BIT2

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
VOID EFIAPI esp_unmount();

/**
 * 打开EFI系统分区内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到和其它错误返回空。其它错误会打印调试输出。
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
 * 根据输入参数初始化AOS系统卷文件系统管理。
 * 
 * @param type       AOS系统卷类型。
 * @param identifier 识别符，用于找到唯一对应的设备。
 * @param params     启动参数。
 * 
 * @return 设备检测状态。
 */
EFI_STATUS EFIAPI asv_init(IN asv_type type,IN VOID* identifier,OUT aos_boot_params* params);

/**
 * 挂载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_mount();

/**
 * 卸载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_unmount();

/**
 * 打开AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * @param mode 打开模式。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
asv_file* EFIAPI asv_open(IN CONST CHAR8* path,IN UINT64 mode);

/**
 * 关闭AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_close(IN asv_file* file);

/**
 * 读取AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 EFIAPI asv_read(IN asv_file* file,OUT VOID* dest,IN UINT64 size);

/**
 * 写入AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 源数组。
 * @param size 期望写入大小。
 * 
 * @return 实际写入大小。
 */
UINT64 EFIAPI asv_write(IN asv_file* file,IN VOID* src,IN UINT64 size);

/**
 * 对AOS系统卷文件指针调整指针位置。偏移量的容许范围是最终结果在0到文件大小上限之间，未写入过的区域自动按0填充。
 * 
 * @param file   文件指针。
 * @param offset 偏移量。
 * @param ref    位置参考。
 * 
 * @return 调整位置状态。
 */
EFI_STATUS EFIAPI asv_reposition(IN asv_file* file,INT64 offset,position_reference ref);

/**
 * 获取AOS系统卷文件指针当前指针位置。
 * 
 * @param file 文件指针。
 * @param pos  位置指针。
 * 
 * @return 获取位置状态。
 */
EFI_STATUS EFIAPI asv_get_position(IN asv_file* file,UINT64* pos);

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * @param size 文件大小指针。
 * 
 * @return 获取文件大小状态。
 */
EFI_STATUS EFIAPI asv_get_size(IN asv_file* file,OUT UINT64* size);

#endif /*__AOS_UEFI_FSM_H__*/
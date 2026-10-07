/**
 * 模块IDT常量定义。
 * @date 2026-09-18
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_IDT_H__
#define __AOS_UEFI_IDT_H__

#include <Uefi.h>

/**
 * 引导IDT数据数组。
 */
extern CONST UINT8 AOS_BOOT_IDT_DATA[];

/**
 * 引导IDT数据大小。
 */
extern CONST UINT64 AOS_BOOT_IDT_DATA_SIZE;

/**
 * 引导中断句柄偏移数组。
 */
extern CONST UINT64 AOS_BOOT_IDT_HANDLER_OFFSET[];

/**
 * IDT向量数目。
 */
#define AOS_IDT_VECTOR_COUNT 256

#endif /*__AOS_UEFI_IDT_H__*/
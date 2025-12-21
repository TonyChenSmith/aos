/**
 * 引导程序使用启动参数结构。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PARAMS_H__
#define __AOS_UEFI_PARAMS_H__

#include "config.h"
#include "lib.h"
#include "version.h"

/**
 * 启动参数使用的数据类型。
 */
#define int16 INT16
#define uint8 UINT8
#define uint16 UINT16
#define uint32 UINT32
#define uint64 UINT64
#define uintn UINTN
#define bool BOOLEAN
#define char16 CHAR16

/**
 * 指向EFIAPI。
 */
#define AOS_EFIAPI EFIAPI

#include "../../../include/boot_params.h"

/**
 * 取消定义。
 */
#undef int16
#undef uint8
#undef uint16
#undef uint32
#undef uint64
#undef uintn
#undef bool
#undef char16
#undef AOS_EFIAPI

#endif /*__AOS_UEFI_PARAMS_H__*/
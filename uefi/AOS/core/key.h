/**
 * 模块X.509公钥证书常量定义。
 * @date 2026-10-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_KEY_H__
#define __AOS_UEFI_KEY_H__

#include <Uefi.h>

/**
 * 模块“aos.kernel”公钥证书数组。
 */
extern CONST UINT8 AOS_KERNEL_PUBLIC_KEY[];

/**
 * 模块“aos.uefi”公钥证书数组。
 */
extern CONST UINT8 AOS_UEFI_PUBLIC_KEY[];

/**
 * 模块“aos.kernel”公钥证书数组大小。
 */
extern CONST UINT64 AOS_KERNEL_PUBLIC_KEY_SIZE;

/**
 * 模块“aos.uefi”公钥证书数组大小。
 */
extern CONST UINT64 AOS_UEFI_PUBLIC_KEY_SIZE;

#endif /*__AOS_UEFI_KEY_H__*/
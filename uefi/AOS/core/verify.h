/**
 * 模块签名验证。
 * @date 2026-10-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_VERIFY_H__
#define __AOS_UEFI_VERIFY_H__

#include "file.h"

/**
 * 验证签名和内核。
 * 
 * @param signature 签名文件句柄。
 * @param kernel    内核文件句柄。
 * 
 * @return 无返回值。
 */
VOID EFIAPI verify_kernel(IN asv_file* signature,IN asv_file* kernel);

#endif /*__AOS_UEFI_VERIFY_H__*/
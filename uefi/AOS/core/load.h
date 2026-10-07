/**
 * 模块内核加载。
 * @date 2026-10-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_LOAD_H__
#define __AOS_UEFI_LOAD_H__

#include "file.h"

/**
 * 加载并映射内核文件。内核必须已完成签名验证。
 * 
 * @param kernel 内核文件句柄。
 * @param base   内核线性基址。
 * 
 * @return 成功加载后返回入口偏移。
 */
UINTN EFIAPI load_kernel(IN asv_file* kernel,IN UINTN base);

#endif /*__AOS_UEFI_LOAD_H__*/
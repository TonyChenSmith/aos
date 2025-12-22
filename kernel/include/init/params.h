/**
 * 内核使用启动参数结构。
 * @date 2025-12-22
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_INIT_PARAMS_H__
#define __AOS_KERNEL_INIT_PARAMS_H__

#include <base.h>

/**
 * EFI使用MS ABI。
 */
#define AOS_EFIAPI __attribute__((ms_abi))

#include "../../../include/boot_params.h"

/**
 * 取消定义。
 */
#undef AOS_EFIAPI

#endif /*__AOS_KERNEL_INIT_PARAMS_H__*/
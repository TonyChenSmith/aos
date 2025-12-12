/**
 * 模块运行环境管理。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_ENV_H__
#define __AOS_UEFI_ENV_H__

#include "params.h"

/**
 * 初始化运行环境。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
EFI_STATUS EFIAPI env_init(IN OUT aos_boot_params* params);

#endif /*__AOS_UEFI_ENV_H__*/
/* 
 * 模块“aos.uefi”运行环境管理声明。
 * 声明了运行环境管理的相关函数。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_ENV_H__
#define __AOS_UEFI_ENV_H__

#include "defines.h"

/* 
 * 在UEFI阶段初始化运行环境，为引导内核准备。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
EFI_STATUS EFIAPI uefi_env_init(IN boot_params* params);

#endif /*__AOS_UEFI_ENV_H__*/
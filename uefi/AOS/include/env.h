/* 
 * 模块“aos.uefi”运行环境管理。
 * 声明了相关的跨文件使用函数。
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
 * 初始化运行环境。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
EFI_STATUS EFIAPI uefi_env_init(IN OUT aos_boot_params* params);

#endif /*__AOS_UEFI_ENV_H__*/
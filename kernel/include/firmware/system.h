/**
 * 内核固件系统相关服务。
 * @date 2026-05-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_FIRMWARE_SYSTEM_H__
#define __AOS_KERNEL_FIRMWARE_SYSTEM_H__

#include "status.h"

/**
 * 通过固件服务将系统冷重置。
 * 
 * @param status 重置状态码。
 * 
 * @return 支持冷重置不返回，不支持返回不支持。
 */
firmware_status firmware_cold_reset(firmware_status status);

/**
 * 通过固件服务将系统热重置。
 * 
 * @param status 重置状态码。
 * 
 * @return 支持热重置不返回，不支持返回不支持。
 */
firmware_status firmware_warm_reset(firmware_status status);

/**
 * 通过固件服务将系统关机。
 * 
 * @param status 重置状态码。
 * 
 * @return 支持关机不返回，不支持返回不支持。
 */
firmware_status firmware_shutdown(firmware_status status);

#endif /*__AOS_KERNEL_FIRMWARE_SYSTEM_H__*/
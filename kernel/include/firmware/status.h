/**
 * 内核固件服务功能状态。
 * @date 2026-05-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_FIRMWARE_STATUS_H__
#define __AOS_KERNEL_FIRMWARE_STATUS_H__

#include <support/type.h>

/**
 * 固件服务状态。
 */
typedef uintn firmware_status;

/**
 * 固件服务正常执行。
 */
#define FIRMWARE_SUCCESS 0

/**
 * 固件服务不支持。
 */
#define FIRMWARE_UNSUPPORTED 1

#endif /*__AOS_KERNEL_FIRMWARE_STATUS_H__*/
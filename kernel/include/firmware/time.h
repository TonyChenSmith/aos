/**
 * 内核固件时间相关服务。
 * @date 2026-05-17
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_FIRMWARE_TIME_H__
#define __AOS_KERNEL_FIRMWARE_TIME_H__

#include <support/time.h>
#include "status.h"

/**
 * 从固件获取当前实时钟时间。
 * 
 * @param time 实时钟时间。
 * 
 * @return 获取时间状态。
 */
firmware_status firmware_get_time(aos_time* time);

/**
 * 设置固件实时钟时间。
 * 
 * @param time 实时钟时间。
 * 
 * @return 设置时间状态。
 */
firmware_status firmware_set_time(aos_time* time);

#endif /*__AOS_KERNEL_FIRMWARE_TIME_H__*/
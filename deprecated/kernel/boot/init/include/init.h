/*
 * 初始化相关定义。
 * @date 2025-01-17
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_INIT_INIT_H__
#define __AOS_BOOT_INIT_INIT_H__

#include "global.h"
#include "params.h"

/*
 * 获取模块函数表。
 * 
 * @return 函数表指针。
 */
typedef void* (*aos_boot_table)(void);

/*获取函数表*/
#define boot_init_table(module) ((aos_boot_table)(module.base+module.entry))()

#endif /*__AOS_BOOT_INIT_INIT_H__*/
/*
 * 模块“aos.uefi”类型声明。定义了在UEFI阶段使用的数据类型。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_TYPE_H__
#define __AOS_UEFI_TYPE_H__

#include <Uefi.h>
/*
 * 启动参数。 
 */
typedef struct _boot_params
{

} boot_params;

/*
 * 启动核蹦床函数，进入后进入启动核流程，并且不返回。
 *
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
typedef VOID (*aos_boot_trampoline)(boot_params* restrict params);

#endif /*__AOS_UEFI_TYPE_H__*/
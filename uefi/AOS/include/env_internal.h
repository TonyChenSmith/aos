/*
 * 模块“aos.uefi”运行环境管理内部声明。
 * 声明了仅用于运行环境管理的相关宏。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_ENV_INTERNAL_H__
#define __AOS_UEFI_ENV_INTERNAL_H__

#include "env.h"

#include <Register/Intel/Cpuid.h>
#include <Register/Amd/Cpuid.h>
#include <Library/MtrrLib.h>

/*
 * 环境使用的MTRR定义类型寄存器。
 */
CONST MSR_IA32_MTRR_DEF_TYPE_REGISTER ENV_MTRR_DEF_TYPE={
    .Bits={
        .Type=CacheWriteBack,
        .E=TRUE,
        .FE=TRUE,
        .Reserved1=0,
        .Reserved2=0,
        .Reserved3=0
    }
};

/*
 * 环境使用的固定MTRR内存类型。
 */
#define ENV_FIXED_MTRR_TYPE ((UINT64)SIGNATURE_64(CacheWriteBack,CacheWriteBack,CacheWriteBack,CacheWriteBack,\
    CacheWriteBack,CacheWriteBack,CacheWriteBack,CacheWriteBack))

#endif /*__AOS_UEFI_ENV_INTERNAL_H__*/
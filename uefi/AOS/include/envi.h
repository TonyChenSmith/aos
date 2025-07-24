/* 
 * 模块“aos.uefi”运行环境管理。
 * 声明了仅在内部使用的的数据类型与宏。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_ENV_INTERNAL_H__
#define __AOS_UEFI_ENV_INTERNAL_H__

#include "env.h"
#include "pmm.h"

#include <Guid/Acpi.h>
#include <Guid/SmBios.h>
#include <IndustryStandard/Acpi.h>
#include <Library/CpuLib.h>
#include <Register/Amd/ArchitecturalMsr.h>
#include <Register/Amd/Cpuid.h>
#include <Register/Intel/ArchitecturalMsr.h>
#include <Register/Intel/Cpuid.h>
#include <Register/Intel/LocalApic.h>

/* 
 * 环境使用的GDT设置。
 */
CONST UINT64 ENV_GDT[]={
    0x0000000000000000ULL, /*NULL*/
    0x00CF9A000000FFFFULL, /*KERNEL_CODE32*/
    0x00CF92000000FFFFULL, /*KERNEL_DATA32*/
    0x00AF9A000000FFFFULL, /*KERNEL_CODE64*/
    0x00CF92000000FFFFULL, /*KERNEL_DATA64*/
    0x00AFFA000000FFFFULL, /*USER_CODE64*/
    0x00CFF2000000FFFFULL, /*USER_DATA64*/
    0x0000000000000000ULL, /*BOUNDARY*/
};

/* 
 * APIC ID链表结构。
 */
typedef struct _env_apic
{
    UINT32 id;
    struct _env_apic* next;
} env_apic;

/* 
 * 环境使用的固定MTRR内存类型。
 */
#define ENV_FIXED_MTRR_TYPE ((UINT64)SIGNATURE_64(MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,\
    MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,\
    MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK))

#endif /*__AOS_UEFI_ENV_INTERNAL_H__*/
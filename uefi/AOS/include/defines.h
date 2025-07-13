/* 
 * 模块“aos.uefi”公用声明。
 * 声明了在UEFI阶段使用的公共宏与数据类型。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_DEFINES_H__
#define __AOS_UEFI_DEFINES_H__

#include "config.h"
#include "includes.h"

/* 
 * 第一轮检查。
 * 主要根据宏用途初步确定范围。
 */

/* 
 * CONFIG_MEMORY_POOL_PAGES检查。
 */
#ifndef CONFIG_MEMORY_POOL_PAGES
#error The macro CONFIG_MEMORY_POOL_PAGES is undefined.
#elif CONFIG_MEMORY_POOL_PAGES<=0
#error The macro CONFIG_MEMORY_POOL_PAGES must be greater than 0.
#elif (CONFIG_MEMORY_POOL_PAGES%64)!=0
#error The macro CONFIG_MEMORY_POOL_PAGES must be a multiple of 64.
#endif /*CONFIG_MEMORY_POOL_PAGES*/

/* 
 * CONFIG_MEMORY_PREALLOCATED_PAGES检查。
 */
#ifndef CONFIG_MEMORY_PREALLOCATED_PAGES
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES is undefined.
#elif CONFIG_MEMORY_PREALLOCATED_PAGES<=0
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES must be greater than 0.
#elif CONFIG_MEMORY_PREALLOCATED_PAGES>CONFIG_MEMORY_POOL_PAGES
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES must be less than or equal to CONFIG_MEMORY_POOL_PAGES.
#endif /*CONFIG_MEMORY_PREALLOCATED_PAGES*/

/* 
 * 启动状态。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _boot_cpu_state
{
    UINT32 la57 :1; /*五级页。*/
    UINT32 fmtrr:1; /*固定MTRR状态。*/
    UINT32 vmtrr:8; /*可变MTRR数目。*/
    UINT32 uc   :3; /*UC索引。*/
    UINT32 wc   :3; /*WC索引。*/
    UINT32 wt   :3; /*WT索引。*/
    UINT32 wp   :3; /*WP索引。*/
    UINT32 wb   :3; /*WB索引。*/
    UINT32 apic :3; /*APIC状态。*/
} boot_cpu_state;

/* 
 * 启动CPU特性。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _boot_cpu_features
{
    UINT32 baseline:8; /*架构基线。*/
    UINT32 vendor  :8; /*厂商。*/
    UINT32 nx      :1; /*NX。*/
    UINT32 page1gb :1; /*1GB大页。*/
    UINT32 la57    :1; /*五级页。*/
    UINT32 xapic   :1; /*xAPIC支持。*/
    UINT32 x2apic  :1; /*x2APIC支持。*/
} boot_cpu_features;

/* 
 * 启动参数。
 * 记录了需要传递到内核的参数。
 */
typedef struct _boot_params
{
    UINTN             bitmap_base; /*位图基址。*/
    UINTN             tlsf_base;   /*TLSF元数据基址。*/
    boot_cpu_features features;    /*CPU特性。*/
    boot_cpu_state    state;       /*CPU状态。*/
    UINTN             acpi;        /*ACPI表。*/
    UINTN             smbios;      /*SMBIOS表。*/
    UINTN             cpus_length; /*处理器数组长度。*/
    UINT32*           cpus;        /*处理器数组。*/
    UINT32            gdt_base;    /*GDT基址。*/
    UINT32            gdt_size;    /*GDT大小。*/
} boot_params;

/* 
 * 启动核蹦床函数，进入后进入启动核流程，并且不返回。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
typedef VOID (*aos_boot_trampoline)(boot_params* restrict params);

/* 
 * x86-64-v1基线。详细定义见SysV ABI。
 */
#define AOS_FEATURE_BASELINE_X86_64_V1 0

/* 
 * x86-64-v2基线。详细定义见SysV ABI。
 */
#define AOS_FEATURE_BASELINE_X86_64_V2 1

/* 
 * Intel处理器。
 */
#define AOS_FEATURE_VENDOR_INTEL 0

/* 
 * AMD处理器。
 */
#define AOS_FEATURE_VENDOR_AMD 1

/* 
 * 未启用APIC。
 */
#define AOS_STATE_NO_APIC 0

/* 
 * 已经启用xAPIC。
 */
#define AOS_STATE_XAPIC 1

/* 
 * 已经启用x2APIC。
 */
#define AOS_STATE_X2APIC 2

#endif /*__AOS_UEFI_DEFINES_H__*/
/* 
 * 模块“aos.uefi”公用定义。
 * 声明了跨模块使用的公共宏与数据类型。
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
 * UEFI_MEMORY_POOL_PAGES检查。
 */
#ifndef UEFI_MEMORY_POOL_PAGES
#error The macro UEFI_MEMORY_POOL_PAGES is undefined.
#elif UEFI_MEMORY_POOL_PAGES<=0
#error The macro UEFI_MEMORY_POOL_PAGES must be greater than 0.
#elif (UEFI_MEMORY_POOL_PAGES%64)!=0
#error The macro UEFI_MEMORY_POOL_PAGES must be a multiple of 64.
#endif /*UEFI_MEMORY_POOL_PAGES*/

/* 
 * UEFI_MEMORY_PREALLOCATED_PAGES检查。
 */
#ifndef UEFI_MEMORY_PREALLOCATED_PAGES
#error The macro UEFI_MEMORY_PREALLOCATED_PAGES is undefined.
#elif UEFI_MEMORY_PREALLOCATED_PAGES<=0
#error The macro UEFI_MEMORY_PREALLOCATED_PAGES must be greater than 0.
#elif UEFI_MEMORY_PREALLOCATED_PAGES>UEFI_MEMORY_POOL_PAGES
#error The macro UEFI_MEMORY_PREALLOCATED_PAGES must be less than or equal to UEFI_MEMORY_POOL_PAGES.
#endif /*UEFI_MEMORY_PREALLOCATED_PAGES*/

/* 
 * UEFI_GRAPHICS_BACKGROUND_COLOR检查。
 */
#ifndef UEFI_GRAPHICS_BACKGROUND_COLOR
#error The macro UEFI_GRAPHICS_BACKGROUND_COLOR is undefined.
#elif UEFI_GRAPHICS_BACKGROUND_COLOR<0
#error The macro UEFI_GRAPHICS_BACKGROUND_COLOR must be greater than or equal to 0.
#elif UEFI_GRAPHICS_BACKGROUND_COLOR>0xFFFFFF
#error The macro UEFI_GRAPHICS_BACKGROUND_COLOR must be less than or equal to 0xFFFFFF.
#endif /*UEFI_GRAPHICS_BACKGROUND_COLOR*/

/* 
 * CPU特性。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _aos_cpu_features
{
    UINT8 magic;    /*魔数。*/
    UINT8 baseline; /*架构基线。*/
    UINT8 vendor;   /*厂商。*/
    UINT8 features; /*CPU特性。*/
} aos_cpu_features;

/* 
 * CPU状态。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 * PAT设置为硬写值，按顺序分别是WB、WT、UC-、UC、WC、WP、UC-、UC。在引导阶段它们不会被使用，因此不定义宏常量。
 */
typedef struct _aos_cpu_state
{
    UINT8 magic; /*魔数。*/
    UINT8 state; /*设备状态。*/
    UINT8 vmtrr; /*可变MTRR数目。*/
    UINT8 apic;  /*APIC状态。*/
} aos_cpu_state;

/* 
 * 图形信息。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _aos_graphics_info
{
    UINT32 hres;      /*水平分辨率。*/
    UINT32 vres;      /*竖直分辨率。*/
    UINTN  fb_base;   /*帧缓冲基址。*/
    UINTN  fb_size;   /*帧缓冲大小。*/
    UINT32 red;       /*红色区掩码。*/
    UINT32 green;     /*绿色区掩码。*/
    UINT32 blue;      /*蓝色区掩码。*/
    UINT32 reserved;  /*保留区掩码。*/
    UINT32 scan_line; /*扫描线长度。*/
} aos_graphics_info;

/* 
 * 启动参数。
 * 记录了需要传递到内核的参数。
 */
typedef struct _aos_boot_params
{
    UINTN             bitmap_base;     /*位图基址。*/
    UINTN             tlsf_base;       /*TLSF元数据基址。*/
    aos_cpu_features  features;        /*CPU特性。*/
    aos_cpu_state     state;           /*CPU状态。*/
    UINTN             acpi;            /*ACPI表。*/
    UINTN             smbios;          /*SMBIOS表。*/
    UINTN             cpus_length;     /*处理器数组长度。*/
    UINT32*           cpus;            /*处理器数组。*/
    UINT32            gdt_base;        /*GDT基址。*/
    UINT32            gdt_size;        /*GDT大小。*/
    aos_graphics_info graphics;        /*图形信息。*/
    UINTN             graphics_device; /*图形设备路径。*/
    UINTN             page_table;      /*内核页表地址。*/
    UINTN             vma_root;        /*线性区根地址。*/
    UINTN             vma_head;        /*线性区头地址。*/
    UINTN             vma_tail;        /*线性区尾地址。*/
} aos_boot_params;

/* 
 * 内核蹦床函数，进入后进入内核，并且不返回。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
typedef VOID (*aos_kernel_trampoline)(aos_boot_params* restrict params);

/* 
 * 特性魔数。
 */
#define AOS_FEATURES_MAGIC 0xAF

/* 
 * x86-64-v1基线。详细定义见SysV ABI。
 */
#define AOS_BASELINE_X86_64_V1 0

/* 
 * x86-64-v2基线。详细定义见SysV ABI。
 */
#define AOS_BASELINE_X86_64_V2 1

/* 
 * Intel处理器。
 */
#define AOS_VENDOR_INTEL 0

/* 
 * AMD处理器。
 */
#define AOS_VENDOR_AMD 1

/* 
 * 特性NX标志位。
 */
#define AOS_FEATURES_NX_BIT BIT0

/* 
 * 特性Page1GB标志位。
 */
#define AOS_FEATURES_PAGE1GB_BIT BIT1

/* 
 * 特性LA57标志位。
 */
#define AOS_FEATURES_LA57_BIT BIT2

/* 
 * 特性xAPIC标志位。
 */
#define AOS_FEATURES_XAPIC_BIT BIT3

/* 
 * 特性x2APIC标志位。
 */
#define AOS_FEATURES_X2APIC_BIT BIT4

/* 
 * 状态魔数。
 */
#define AOS_STATE_MAGIC 0xAE

/* 
 * 状态LA57标志位。
 */
#define AOS_STATE_LA57_BIT BIT0

/* 
 * 状态Fixed MTRR标志位。
 */
#define AOS_STATE_FIXED_MTRR_BIT BIT1

/* 
 * 未启用APIC。
 */
#define AOS_APIC_NO_APIC 0

/* 
 * 已经启用xAPIC。
 */
#define AOS_APIC_XAPIC 1

/* 
 * 已经启用x2APIC。
 */
#define AOS_APIC_X2APIC 2

#endif /*__AOS_UEFI_DEFINES_H__*/
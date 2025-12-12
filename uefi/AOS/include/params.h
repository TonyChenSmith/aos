/**
 * 模块启动参数定义。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PARAMS_H__
#define __AOS_UEFI_PARAMS_H__

#include "config.h"
#include "lib.h"
#include "version.h"

/**
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

/**
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

/**
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

/**
 * 设备路径结点。
 * 具体结构定义参考UEFI规范。
 */
typedef struct _aos_efi_device_path
{
    UINT8 type;      /*类型。*/
    UINT8 sub_type;  /*子类型。*/
    UINT8 length[2]; /*长度。*/
    UINT8 data[];    /*数据数组。*/
} aos_efi_device_path;

/**
 * 线性区。这里使用了简单的双向链表结构用于传递信息。
 * 红黑树版本的在内核再实现。
 */
typedef struct _aos_boot_vma aos_boot_vma;

struct _aos_boot_vma
{
    aos_boot_vma* prev;  /*前一线性区。*/
    aos_boot_vma* next;  /*后一线性区。*/
    UINTN         start; /*开始地址。*/
    UINTN         end;   /*结束地址。*/
    UINT64        flags; /*标志。*/
};

/**
 * 启动参数。
 * 记录了需要传递到内核的参数。
 */
typedef struct _aos_boot_params
{
    UINTN                bpool_base;      /*引导内存池基址。*/
    UINTN                bpool_pages;     /*引导内存池页数。*/
    UINTN                kpool_base;      /*内核内存池基址。*/
    UINTN                kpool_pages;     /*内核内存池页数。*/
    UINTN                ppool_base;      /*页表内存池基址。*/
    UINTN                ppool_pages;     /*页表内存池页数。*/
    UINT8*               bitmap;          /*页表内存池位图地址。*/
    UINTN                bitmap_length;   /*页表内存池位图长度。*/
    aos_cpu_features     features;        /*CPU特性。*/
    aos_cpu_state        state;           /*CPU状态。*/
    UINTN                acpi;            /*ACPI表。*/
    UINTN                smbios;          /*SMBIOS表。*/
    UINTN                cpus_length;     /*处理器数组长度。*/
    UINT32*              cpus;            /*处理器数组。*/
    UINT32               gdt_base;        /*GDT基址。*/
    UINT32               gdt_size;        /*GDT大小。*/
    aos_graphics_info    graphics;        /*图形信息。*/
    aos_efi_device_path* graphics_device; /*图形设备路径。*/
    UINTN                page_table;      /*内核页表地址。*/
    aos_boot_vma*        vma_head;        /*线性区头地址。*/
    aos_boot_vma*        vma_tail;        /*线性区尾地址。*/
} aos_boot_params;

/**
 * 内核蹦床函数，进入后进入内核，并且不返回。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
typedef VOID (*aos_kernel_trampoline)(aos_boot_params* restrict params);

/**
 * 特性魔数。
 */
#define AOS_FEATURES_MAGIC 0xAF

/**
 * x86-64-v1基线。详细定义见SysV ABI。
 */
#define AOS_BASELINE_X86_64_V1 0

/**
 * x86-64-v2基线。详细定义见SysV ABI。
 */
#define AOS_BASELINE_X86_64_V2 1

/**
 * Intel处理器。
 */
#define AOS_VENDOR_INTEL 0

/**
 * AMD处理器。
 */
#define AOS_VENDOR_AMD 1

/**
 * 特性NX标志位。
 */
#define AOS_FEATURES_NX BIT0

/**
 * 特性Page1GB标志位。
 */
#define AOS_FEATURES_PAGE1GB BIT1

/**
 * 特性LA57标志位。
 */
#define AOS_FEATURES_LA57 BIT2

/**
 * 特性xAPIC标志位。
 */
#define AOS_FEATURES_XAPIC BIT3

/**
 * 特性x2APIC标志位。
 */
#define AOS_FEATURES_X2APIC BIT4

/**
 * 状态魔数。
 */
#define AOS_STATE_MAGIC 0xAE

/**
 * 状态LA57标志位。
 */
#define AOS_STATE_LA57 BIT0

/**
 * 状态Fixed MTRR标志位。
 */
#define AOS_STATE_FIXED_MTRR BIT1

/**
 * 未启用APIC。
 */
#define AOS_APIC_NO_APIC 0

/**
 * 已经启用xAPIC。
 */
#define AOS_APIC_XAPIC 1

/**
 * 已经启用x2APIC。
 */
#define AOS_APIC_X2APIC 2

/**
 * 线性区内存类型掩码。
 */
#define AOS_BOOT_VMA_TYPE_MASK MAX_UINT8

/**
 * 线性区无缓存类型。
 */
#define AOS_BOOT_VMA_TYPE_UC 0

/**
 * 线性区写合并类型。
 */
#define AOS_BOOT_VMA_TYPE_WC 1

/**
 * 线性区写直通类型。
 */
#define AOS_BOOT_VMA_TYPE_WT 2

/**
 * 线性区写回类型。
 */
#define AOS_BOOT_VMA_TYPE_WB 3

/**
 * 线性区写保护类型。
 */
#define AOS_BOOT_VMA_TYPE_WP 4

/**
 * 线性区无缓存减类型。
 */
#define AOS_BOOT_VMA_TYPE_UCM 5

/**
 * 线性区可读标志。
 */
#define AOS_BOOT_VMA_READ BIT8

/**
 * 线性区可写标志。
 */
#define AOS_BOOT_VMA_WRITE BIT9

/**
 * 线性区可执行标志。
 */
#define AOS_BOOT_VMA_EXECUTE BIT10

/**
 * 线性区读写可执行掩码。
 */
#define AOS_BOOT_VMA_RWX_MASK (AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE)

/**
 * 线性区用户级标志。
 */
#define AOS_BOOT_VMA_USER BIT11

/**
 * 线性区全局标志。
 */
#define AOS_BOOT_VMA_GLOBAL BIT12

/**
 * 线性区已分配标志。
 */
#define AOS_BOOT_VMA_ALLOCATED BIT13

/**
 * 线性区大页标志。
 */
#define AOS_BOOT_VMA_HUGEPAGE BIT14

#endif /*__AOS_UEFI_PARAMS_H__*/
/**
 * 模块启动环境管理所需的不公开内容。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_ENV_INTERNAL_H__
#define __AOS_UEFI_ENV_INTERNAL_H__

#include "env.h"
#include "mem.h"

#include <Guid/Acpi.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/SmBios.h>
#include <IndustryStandard/Acpi.h>
#include <Library/BaseCryptLib.h>
#include <Library/CpuLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/RngLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>
#include <Register/Amd/ArchitecturalMsr.h>
#include <Register/Amd/Cpuid.h>
#include <Register/Intel/ArchitecturalMsr.h>
#include <Register/Intel/Cpuid.h>
#include <Register/Intel/LocalApic.h>

/**
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

/**
 * 位图使用的掩码。
 */
CONST UINT8 ENV_BITMAP_MASK[]={0,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE};

/**
 * APIC ID链表结构。
 */
typedef struct _env_apic
{
    UINT32 id;              /*APIC ID。*/
    struct _env_apic* next; /*下一个链表节点。*/
} env_apic;

/**
 * EDID结构，下面仅对必要条目进行注释，详细解释看VESA文档。
 */
#pragma pack(1)
typedef struct _env_edid
{
    /*头部*/
    UINT8  header[8];

    /*厂家和生产厂商ID信息*/
    UINT16 id_manufacturer_name;
    UINT16 id_product_code;
    UINT32 id_serial_number;
    UINT8  week_of_manufacture;
    UINT8  year_of_manufacture;

    /*EDID结构版本和修订号*/
    UINT8  version_number;
    UINT8  revision_number;

    /*基本显示参数和功能*/
    UINT8  video_input_definition;
    UINT8  horizontal_screen_size;
    UINT8  vertical_screen_size;
    UINT8  gamma;
    UINT8  feature_support;

    /*颜色*/
    UINT8  red_green_low;
    UINT8  blue_white_low;
    UINT8  red_x_high;
    UINT8  red_y_high;
    UINT8  green_x_high;
    UINT8  green_y_high;
    UINT8  blue_x_high;
    UINT8  blue_y_high;
    UINT8  white_x_high;
    UINT8  white_y_high;

    /*既定时序*/
    UINT8  established_timings_1;
    UINT8  established_timings_2;
    UINT8  manufacturer_reserved_timings;

    /*标准时序*/
    UINT16 standard_timings[8];

    /*18字节数据块，仅在像素时钟有效情况下下面才有效*/
    struct {
        UINT16 pixel_clock;
        UINT8  horizontal_active_low;
        UINT8  horizontal_blanking_low;
        UINT8  horizontal_high;
        UINT8  vertical_active_low;
        UINT8  vertical_blanking_low;
        UINT8  vertical_high;
        UINT8  horizontal_sync_offset_low;
        UINT8  horizontal_sync_pulse_width_low;
        UINT8  vertical_sync_low;
        UINT8  sync_high;
        UINT8  horizontal_image_size_low;
        UINT8  vertical_image_size_low;
        UINT8  image_size_high;
        UINT8  horizontal_border;
        UINT8  vertical_border;
        UINT8  flags;
    } detailed_timings[4];

    /*扩展块计数*/
    UINT8  extension_block_count;

    /*校验码*/
    UINT8  checksum;
} env_edid;
#pragma pack()

/**
 * 图形像素。
 */
typedef union _env_graphics_pixel
{
    UINT8  data[4]; /*像素数据。*/
    UINT32 pixel;   /*像素值。*/
} env_graphics_pixel;

/**
 * 环境使用的固定MTRR内存类型。
 */
#define ENV_FIXED_MTRR_TYPE ((UINT64)SIGNATURE_64(MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,\
    MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,\
    MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK))

/**
 * VGA水平分辨率。
 */
#define ENV_VGA_HORIZONTAL_RESOLUTION 640

/**
 * VGA竖直分辨率。
 */
#define ENV_VGA_VERTICAL_RESOLUTION 480

/**
 * SVGA水平分辨率。
 */
#define ENV_SVGA_HORIZONTAL_RESOLUTION 800

/**
 * SVGA竖直分辨率。
 */
#define ENV_SVGA_VERTICAL_RESOLUTION 600

/**
 * 背景色红值。
 */
#define ENV_BACKGROUND_RED ((CONFIG_GRAPHICS_BACKGROUND_COLOR>>16)&MAX_UINT8)

/**
 * 背景色绿值。
 */
#define ENV_BACKGROUND_GREEN ((CONFIG_GRAPHICS_BACKGROUND_COLOR>>8)&MAX_UINT8)

/**
 * 背景色蓝值。
 */
#define ENV_BACKGROUND_BLUE (CONFIG_GRAPHICS_BACKGROUND_COLOR&MAX_UINT8)

#endif /*__AOS_UEFI_ENV_INTERNAL_H__*/
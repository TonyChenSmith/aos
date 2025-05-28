/*
 * 图像操作相关定义。设置VGA兼容输出相关。
 * @date 2025-01-25
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_GRAPHICS_H__
#define __AOS_UEFI_GRAPHICS_H__

#include <Guid/ConsoleOutDevice.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>

#include "params.h"

#pragma pack(1)

/*EDID结构，下面仅对必要条目进行注释，详细解释看VESA文档*/
typedef struct _edid
{
	/*头部*/
	UINT8 header[8];

	/*厂家和生产厂商ID信息*/
	UINT16 id_manufacturer_name;
	UINT16 id_product_code;
	UINT32 id_serial_number;
	UINT8 week_of_manufacture;
	UINT8 year_of_manufacture;

	/*EDID结构版本和修订号*/
	UINT8 version_number;
	UINT8 revision_number;

	/*基本显示参数和功能*/
	UINT8 video_input_definition;
	UINT8 horizontal_screen_size;
	UINT8 vertical_screen_size;
	UINT8 gamma;
	UINT8 feature_support;

	/*颜色*/
	UINT8 red_green_low;
	UINT8 blue_white_low;
	UINT8 red_x_high;
	UINT8 red_y_high;
	UINT8 green_x_high;
	UINT8 green_y_high;
	UINT8 blue_x_high;
	UINT8 blue_y_high;
	UINT8 white_x_high;
	UINT8 white_y_high;

	/*既定时序*/
	UINT8 established_timings_1;
	UINT8 established_timings_2;
	UINT8 manufacturer_reserved_timings;

	/*标准时序*/
	UINT16 standard_timings[8];

	/*18字节数据块*/
	/*仅在像素时钟有效情况下下面才有效*/
	struct {
		UINT16 pixel_clock;
		UINT8 horizontal_active_low;
		UINT8 horizontal_blanking_low;
		UINT8 horizontal_high;
		UINT8 vertical_active_low;
		UINT8 vertical_blanking_low;
		UINT8 vertical_high;
		UINT8 horizontal_sync_offset_low;
		UINT8 horizontal_sync_pulse_width_low;
		UINT8 vertical_sync_low;
		UINT8 sync_high;
		UINT8 horizontal_image_size_low;
		UINT8 vertical_image_size_low;
		UINT8 image_size_high;
		UINT8 horizontal_border;
		UINT8 vertical_border;
		UINT8 flags;
	} detailed_timings[4];

	/*扩展块计数*/
	UINT8 extension_block_count;

	/*校验码*/
	UINT8 checksum;
} EDID;

#pragma pack()

#endif /*__AOS_UEFI_GRAPHICS_H__*/
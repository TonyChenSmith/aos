/*
 * AOS Loader定义头文件。
 * 2024-05-28创建。 
 */
#ifndef __AOS_DEFINE_H__
#define __AOS_DEFINE_H__

/*头文件*/

/*标准头文件*/
#include <Uefi.h>

/*宏定义与类型定义*/

/*图像*/

/*图像信息，用于后续没有特有驱动时输出。*/
typedef struct
{
	/*水平分辨率。x轴范围。*/
	UINT32 horizontal_resolution;
	/*竖直分辨率。y轴范围。*/
	UINT32 vertical_resolution;
	/*帧缓存区基址。*/
	UINT64 frame_buffer_base;
	/*帧缓存区大小。*/
	UINT64 frame_buffer_size;
	/*红区位。*/
	UINT32 red_mask;
	/*绿区位。*/
	UINT32 green_mask;
	/*蓝区位。*/
	UINT32 blue_mask;
	/*保留区（透明度）位。*/
	UINT32 reserved_mask;
	/*扫描线长度。小于等下水平分辨率。*/
	UINT32 scan_line_length;
	/*模式。直接写入模式0，纯Blt为1.其余功能保留。*/
	UINT32 mode;
} AOS_GRAPHICS_INFO;

/*EDID结构*/
typedef struct
{
	/*头部*/
	UINT8 header[8];

	/*Vendor & Product Identification*/
	UINT16 id_manufacturer_name;
	UINT16 id_product_code;
	UINT32 id_serial_number;
	UINT8 week_of_manufacture;
	UINT8 year_of_manufacture;

	/*EDID Structure Version & Revision*/
	UINT8 version_number;
	UINT8 revision_number;

	/*Basic Display Parameters & Features*/
	UINT8 video_input_definition;
	UINT8 horizontal_screen_size;
	UINT8 vertical_screen_size;
	UINT8 gamma;
	UINT8 feature_support;

	/*Color Characteristics*/
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

	/*Established Timings*/
	UINT8 established_timings_1;
	UINT8 established_timings_2;
	UINT8 manufacturer_reserved_timings;

	/*Standard Timings*/
	UINT16 standard_timings[8];

	/*18 Byte Data Blocks*/
	struct {
		UINT16 pixel_clock;
		/*仅在像素时钟有效情况下下面才有效。*/
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

	/*Extension Block Count N*/
	UINT8 extension_block_count;

	/*Checksum*/
	UINT8 checksum;
} AOS_EDID;

/*内存*/

/*页面常量*/
/*AOS的标准页为4k页。*/
#define AOS_PAGE_SIZE SIZE_4KB
#define AOS_PAGE_MASK EFI_PAGE_MASK
#define AOS_PAGE_SHIFT EFI_PAGE_SHIFT

/*AOS尺寸页数互相转换。*/
#define aos_pages_to_size(pages) EFI_PAGES_TO_SIZE(pages)
#define aos_size_to_pages(size) EFI_SIZE_TO_PAGES(size)

/*定义AOS内存种类枚举。从紧凑和易读角度，命名方式保留UEFI的样式。*/
typedef enum _AOS_MEMORY_TYPE
{
	/*不被使用，保留区域。未知区域都可以被设置为保留区域。*/
	AOSReservedMemory,
	/*保留给ACPI表。*/
	AOSACPIReclaimMemory,
	/*保留给ACPI固件。*/
	AOSACPIMemoryNVS,
	/*MMIO。*/
	AOSMemoryMappedIO,
	/*NNIO转换到IO区域过程中的保留MMIO。*/
	AOSMemoryMappedIOPortSpace,
	/*固件保留给处理器微架构代码的区域。*/
	AOSPalCode,
	/*非易失性空闲内存。*/
	AOSPersistentMemory,
	/*常规空闲内存。*/
	AOSConventionalMemory,
	/*未接受内存类型，这里显式保留类型，但依旧视为不存在，其余未映射区域视为不存在。*/
	AOSUnacceptedMemory,
	/*以下为独特内存类型*/
	/*UEFI RT代码区域。*/
	AOSUEFIRuntimeCode,
	/*UEFI RT数据区域。*/
	AOSUEFIRuntimeData,
	/*内核代码。*/
	AOSKernelCode,
	/*内核数据。*/
	AOSKernelData,
	/*CPU数据。内核级别，与一般内核数据区分一下。一般是给段表、中断表和页表使用。*/
	AOSCpuData,
	/*用户代码。*/
	AOSUserCode,
	/*用户数据。*/
	AOSUserData,
	/*UEFI BT代码区域。启动载入代码也计入遗留代码。在后面将会清理该空间数据。*/
	AOSUEFILegacyCode,
	/*UEFI BT数据区域。启动载入数据也计入遗留数据。在后面将会清理该空间数据。*/
	AOSUEFILegacyData,
} AOS_MEMORY_TYPE;

/*AOS内存描述符*/
typedef struct _AOS_PHYSICAL_MEMORY_DESCRIPTOR
{
	UINT64 type;	  /*内存类型。*/
	UINT64 base;	  /*物理基址。*/
	UINT64 amount;	  /*页面数量。*/
	UINT64 attribute; /*内存属性。*/
} AOS_PHYSICAL_MEMORY_DESCRIPTOR;

/*启动参数*/
typedef struct _AOS_BOOT_PARAMS
{
	/*图像信息*/
	AOS_GRAPHICS_INFO graphics_info;
} AOS_BOOT_PARAMS;

#endif /*__AOS_DEFINE_H__*/
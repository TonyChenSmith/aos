/*
 * AOS Loader定义头文件。
 * 2024-05-28创建。 
 */
#ifndef __AOS_DEFINE_H__
#define __AOS_DEFINE_H__

#include <Uefi.h>
#include <Guid/Acpi.h>
#include <Guid/SmBios.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#pragma pack(1)

/*图像*/

/*EDID结构*/
typedef struct
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
	struct {
		UINT16 pixel_clock;
		/*仅在像素时钟有效情况下下面才有效*/
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
} AOS_EDID;

/*PCI QWORD表*/
typedef struct
{
	/*QWORD描述表*/
	UINT8 qword_desciptor; 			   /*表类型，应为0x8A*/
	UINT16 qword_length; 			   /*长度，应为0x2B*/
	UINT8 resource_type; 			   /*资源类型*/
	UINT8 general_flags; 			   /*可变标志*/
	UINT8 type_specific_flags; 		   /*类型特殊标志*/

	UINT64 address_space_granularity;  /*地址精度*/
	UINT64 address_range_minimum;	   /*地址范围最小值*/
	UINT64 address_range_maximum;	   /*地址范围最大值*/
	UINT64 address_translation_offset; /*地址翻译偏移*/
	UINT64 address_length;			   /*地址长度*/
} PCI_QWORD_TABLE;

/*PCI结尾表*/
typedef struct
{
	/*结尾表*/
	UINT8 end_tag;  /*结尾标志，应为0x79*/
	UINT8 checksum;	/*校验码*/
} PCI_END_TABLE;

#pragma pack()

/*图像信息，用于后续没有特有驱动时输出*/
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
	/*扫描线长度。小于等于水平分辨率*/
	UINT32 scan_line_length;
} AOS_GRAPHICS_INFO;

/*范围表示，闭区间*/
typedef struct
{
	UINTN mix; /*最小值*/
	UINTN max; /*最大值*/
} PCI_RANGE;

/*BAR描述*/
typedef struct
{
	UINT64 supports; /*支持属性*/
	UINT8 resource; /*资源类型,4代表不存在该BAR*/
	UINT8 general; /*通用标志*/
	UINT8 type_specific; /*类型特有标志*/
	UINTN granularity; /*粒度*/
	UINTN min; /*最小值*/
	UINTN max; /*最大值*/
	UINTN offset; /*偏移*/
	UINTN length; /*长度*/ 
} PCI_BAR;

/*PCI根桥信息*/
typedef struct
{
	UINTN segment; /*段号*/
	UINT64 supports; /*支持属性*/
	UINT64 attributes; /*属性，具体定义查看UEFI规范相关章节。*/
	UINTN length; /*总线范围数组长度*/
	PCI_RANGE buses[]; /*总线范围*/
} PCI_ROOT_BRIDGE_INFO;

/*PCI设备信息*/
typedef struct
{
	UINTN segment; /*段号*/
	UINTN bus; /*总线*/
	UINTN device; /*设备*/
	UINTN function; /*功能*/
	UINT64 supports; /*支持属性*/
	UINT64 attributes; /*属性，具体定义查看UEFI规范相关章节。*/
	PCI_BAR bars[6]; /*BAR信息*/
} PCI_DEVICE_INFO;

/*CPU信息*/
typedef struct
{
	UINT32 max_processors; /*CPU拥有的逻辑处理器核心数*/
	BOOLEAN level5; /*是否支持五级页*/
	BOOLEAN page1gb;		   /*1GB大页*/
	BOOLEAN pge;			   /*全局页*/
	BOOLEAN erms; /*增强rep movs*/
} AOS_CPU_INFO;

/*段、分页和中断信息*/
typedef struct
{
	VOID* max_free; /*最大自由区，用于内核进行内存分配*/
	UINTN max_pages; /*最大自由区页面数目*/
	AOS_CPU_INFO cpu_info;
	IA32_DESCRIPTOR gdtr; /*GDT信息*/
	IA32_DESCRIPTOR idtr; /*IDT信息*/
	UINTN ist; /*IST基址，这里为中断异常栈*/
	UINTN ist_length; /*IST长度*/
	UINTN tss; /*TSS基址，这里为每个核心的TSS集合*/
	UINTN tss_length; /*TSS总长*/
	EFI_MEMORY_DESCRIPTOR* memmap; /*内存图*/
	UINTN entry_size; /*单项的大小*/
	UINTN memmap_length; /*内存图长度，字节计数*/
} AOS_ENV_CONFIG;

/*引导程序信息*/
typedef struct
{
	UINTN base; /*程序基址，物理地址*/
	UINTN entry; /*入口偏移*/
	UINTN pages; /*页面数目*/
} AOS_BOOTSTRAP_CODE_INFO;

/*内存*/

/*页面常量*/
/*AOS的标准页为4k页*/
#define AOS_PAGE_SIZE SIZE_4KB
#define AOS_PAGE_MASK EFI_PAGE_MASK
#define AOS_PAGE_SHIFT EFI_PAGE_SHIFT

/*AOS尺寸页数互相转换*/
#define aos_pages_to_size(pages) EFI_PAGES_TO_SIZE(pages)
#define aos_size_to_pages(size) EFI_SIZE_TO_PAGES(size)

/*计算结构体相对偏移，字节量*/
#define aos_offset_of(type,field) OFFSET_OF(type,field)

/*预先申请空间大小，包括预分配内存池和运行栈，内核文件为独立申请*/
/*目前暂时设置为各8MB*/
#define AOS_MEMORY_POOL_PAGES aos_size_to_pages(SIZE_8MB)
#define AOS_STACK_PAGES aos_size_to_pages(SIZE_8MB)
/*单个IST大小*/
#define AOS_IST_PAGES aos_size_to_pages(SIZE_8KB)

/*表大小*/
#define AOS_GDT_PAGES aos_size_to_pages(SIZE_64KB)
#define AOS_KERNEL_IDT_PAGES aos_size_to_pages(SIZE_4KB)

/*启动模块数目*/
#define AOS_BOOT_MODULE_COUNT 3

/*启动参数*/
typedef struct _AOS_BOOT_PARAMS
{
	/*存储池信息*/
	VOID* pool; /*内存池基址*/
	UINTN pool_length; /*内存池长度*/
	VOID* stack; /*栈基址*/
	UINTN stack_length; /*栈长度*/

	/*PCI设备信息*/
	PCI_ROOT_BRIDGE_INFO* root_bridges; /*根桥信息数组*/
	UINTN root_bridge_length; /*根桥数目*/
	PCI_DEVICE_INFO* devices; /*设备信息数组*/
	UINTN device_length; /*设备数目*/

	/*图像信息*/
	AOS_GRAPHICS_INFO graphics_info; /*图像信息数据结构*/

	/*环境*/
	AOS_ENV_CONFIG env; /*环境信息*/

	/*系统表*/
	VOID* acpi;
	VOID* smbios;
	VOID* smbios3;
	EFI_RUNTIME_SERVICES* runtime;

	/*引导设备*/
	VOID* boot_device;
	/*引导程序*/
	AOS_BOOTSTRAP_CODE_INFO modules[AOS_BOOT_MODULE_COUNT];
} AOS_BOOT_PARAMS;

/*启动参数*/
extern AOS_BOOT_PARAMS boot_params;

#endif /*__AOS_DEFINE_H__*/
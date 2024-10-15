/*
 * AOS Loader定义头文件。
 * 2024-05-28创建。 
 */
#ifndef __AOS_DEFINE_H__
#define __AOS_DEFINE_H__

/*头文件*/

/*头文件*/
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

/*宏定义与类型定义*/

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
	/*扫描线长度。小于等下水平分辨率*/
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

/*段、分页和中断信息*/
typedef struct
{
	BOOLEAN level5; /*是否五级页*/
	VOID* physical_page_table; /*物理页表，即1:1映射*/
	VOID* kernel_page_table; /*内核页表，内核运行时使用的页表*/
	IA32_DESCRIPTOR gdtr; /*GDT信息*/
	IA32_DESCRIPTOR idtr; /*IDT信息*/
} AOS_ENV_CONFIG;

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
#define AOS_MEMORY_POOL_PAGES aos_size_to_pages(0x800000)
#define AOS_STACK_PAGES aos_size_to_pages(0x800000)

/*定义AOS内存种类枚举。从紧凑和易读角度，命名方式保留UEFI的样式*/
typedef enum _AOS_MEMORY_TYPE
{
	/*不被使用，保留区域。未知区域都可以被设置为保留区域*/
	AOSReservedMemory,
	/*保留给ACPI表*/
	AOSACPIReclaimMemory,
	/*保留给ACPI固件*/
	AOSACPIMemoryNVS,
	/*MMIO*/
	AOSMemoryMappedIO,
	/*NNIO转换到IO区域过程中的保留MMIO*/
	AOSMemoryMappedIOPortSpace,
	/*固件保留给处理器微架构代码的区域*/
	AOSPalCode,
	/*非易失性空闲内存*/
	AOSPersistentMemory,
	/*常规空闲内存*/
	AOSConventionalMemory,
	/*未接受内存类型，这里显式保留类型，但依旧视为不存在，其余未映射区域视为不存在*/
	AOSUnacceptedMemory,
	/*以下为独特内存类型*/
	/*UEFI RT代码区域*/
	AOSUEFIRuntimeCode,
	/*UEFI RT数据区域*/
	AOSUEFIRuntimeData,
	/*页表*/
	AOSPageTable,
	/*GDT*/
	AOSGDT,
	/*IDT*/
	AOSIDT,
	/*内核代码*/
	AOSKernelCode,
	/*内核数据*/
	AOSKernelData,
	/*UEFI BT代码区域*/
	AOSUEFILegacyCode,
	/*UEFI BT数据区域*/
	AOSUEFILegacyData,
	/*UEFI 加载器代码区域。为减少出问题的可能性，不移除*/
	AOSUEFILoaderCode,
	/*UEFI 加载器数据区域。为减少出问题的可能性，不移除*/
	AOSUEFILoaderData,
} AOS_MEMORY_TYPE;

/*AOS内存描述符*/
typedef struct _AOS_PHYSICAL_MEMORY_DESCRIPTOR
{
	UINT64 type;	  /*内存类型*/
	UINT64 base;	  /*物理基址*/
	UINT64 amount;	  /*页面数量*/
	UINT64 attribute; /*内存属性*/
} AOS_PHYSICAL_MEMORY_DESCRIPTOR;

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
} AOS_BOOT_PARAMS;

/*启动参数*/
extern AOS_BOOT_PARAMS boot_params;

#endif /*__AOS_DEFINE_H__*/
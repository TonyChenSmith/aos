/**
 * 系统保留内容。算废弃代码了。
 * @date 2025-12-20
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_RESERVED_H__
#define __AOS_RESERVED_H__

/**
 * EFI内存类型。
 */
typedef enum _aos_efi_memory_type
{
    efi_reserved_memory_type,                /*保留内存。*/
    efi_loader_code,                         /*加载器代码。*/
    efi_loader_data,                         /*加载器数据。*/
    efi_boot_services_code,                  /*引导服务代码。*/
    efi_boot_services_data,                  /*引导服务数据。*/
    efi_runtime_services_code,               /*运行时服务代码。*/
    efi_runtime_services_data,               /*运行时服务数据。*/
    efi_conventional_memory,                 /*未分配内存。*/
    efi_unusable_memory,                     /*不可用内存。*/
    efi_acpi_reclaim_memory,                 /*ACPI表内存。*/
    efi_acpi_memory_nvs,                     /*ACPI固件保留内存。*/
    efi_memory_mapped_io,                    /*内存映射输入输出。*/
    efi_memory_mapped_io_port_space,         /*内存映射输入输出。*/
    efi_pal_code,                            /*固件保留处理器代码。*/
    efi_persistent_memory,                   /*未分配非易失性内存。*/
    efi_unaccepted_memory_type,              /*未接受内存。*/
    efi_max_memory_type,                     /*最大内存类型。*/
    memory_type_oem_reserved_min=0x70000000, /*OEM保留最小值。*/
    memory_type_oem_reserved_max=0x7FFFFFFF, /*OEM保留最大值。*/
    memory_type_os_reserved_min=0x80000000,  /*OS保留最小值。*/
    memory_type_os_reserved_max=0xFFFFFFFF   /*OS保留最大值。*/
} aos_efi_memory_type;

/**
 * EFI内存属性。
 */
#define AOS_EFI_MEMORY_UC 0x1ULL
#define AOS_EFI_MEMORY_WC 0x2ULL
#define AOS_EFI_MEMORY_WT 0x4ULL
#define AOS_EFI_MEMORY_WB 0x8ULL
#define AOS_EFI_MEMORY_UCE 0x10ULL
#define AOS_EFI_MEMORY_WP 0x1000ULL
#define AOS_EFI_MEMORY_RP 0x2000ULL
#define AOS_EFI_MEMORY_XP 0x4000ULL
#define AOS_EFI_MEMORY_RO 0x20000ULL
#define AOS_EFI_MEMORY_NV 0x8000ULL
#define AOS_EFI_MEMORY_MORE_RELIABLE 0x10000ULL
#define AOS_EFI_MEMORY_SP 0x40000ULL
#define AOS_EFI_MEMORY_CPU_CRYPTO 0x80000ULL
#define AOS_EFI_MEMORY_HOT_PLUGGABLE 0x100000ULL
#define AOS_EFI_MEMORY_RUNTIME 0x8000000000000000ULL
#define AOS_EFI_MEMORY_ISA_VALID 0x4000000000000000ULL
#define AOS_EFI_MEMORY_ISA_MASK 0xFFFF00000000000ULL

/**
 * EFI时间常量。
 */
#define AOS_EFI_TIME_ADJUST_DAYLIGHT 0x1
#define AOS_EFI_TIME_IN_DAYLIGHT 0x2
#define AOS_EFI_UNSPECIFIED_TIMEZONE 0x7FF

/**
 * EFI胶囊标志。
 */
#define AOS_EFI_CAPSULE_FLAGS_PERSIST_ACROSS_RESET 0x10000
#define AOS_EFI_CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE 0x20000
#define AOS_EFI_CAPSULE_FLAGS_INITIATE_RESET 0x40000

/**
 * 对标准色彩值按像素格式要求缩放。
 * 
 * @param value 单色色值。
 * @param width 目标位宽。
 * 
 * @return 缩放后的色彩值。
 */
STATIC UINT32 EFIAPI env_scale_color(IN UINT8 value,IN UINT8 width)
{
    if(width==8)
    {
        return value;
    }
    else
    {
        UINT64 max=(1ULL<<width)-1;
        return (UINT32)(((value*max+127)/255)&max);
    }
}

/**
 * 在指定区域内填充颜色。调用者有责任保证参数合理。
 * 
 * @param pixel     像素颜色。
 * @param size      像素大小。
 * @param scan_line 扫面线像素数。
 * @param fb_base   帧缓冲基址。
 * @param x         起点横坐标。
 * @param y         起点纵坐标。
 * @param w         区域宽度。
 * @param h         区域高度。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI env_fill_color(IN env_graphics_pixel pixel,IN UINT8 size,IN UINT32 scan_line,IN UINTN fb_base,
    IN UINT32 x,IN UINT32 y,IN UINT32 w,IN UINT32 h)
{
    if(size==4)
    {
        UINT32* line=(UINT32*)(fb_base+((y*scan_line+x)<<2));
        for(UINTN j=0;j<h;j++)
        {
            SetMem32(line,w<<2,pixel.pixel);
            line=(UINT32*)((UINTN)line+(scan_line<<2));
        }
    }
    else if(size<4&&size>0)
    {
        UINT8* line=(UINT8*)(fb_base+((y*scan_line+x)*size));
        for(UINTN j=0;j<h;j++)
        {
            for(UINTN i=0;i<w;i++)
            {
                UINTN base=i*size;
                for(UINT8 k=0;k<size;k++)
                {
                    line[base+k]=pixel.data[k];
                }
            }
            line=(UINT8*)((UINTN)line+scan_line*size);
        }
    }
}

/**
 * 强制填充图形界面背景。要求为一个可以表示真假的数值。
 */
#define CONFIG_FORCE_FILL_GRAPHICS_BACKGROUND FALSE

/**
 * 图形界面背景色。要求范围为[0,0xFFFFFF]。
 */
#define CONFIG_GRAPHICS_BACKGROUND_COLOR 0x191970

/**
 * CONFIG_GRAPHICS_BACKGROUND_COLOR检查。
 */
#ifndef CONFIG_GRAPHICS_BACKGROUND_COLOR
#error The macro CONFIG_GRAPHICS_BACKGROUND_COLOR is undefined.
#elif CONFIG_GRAPHICS_BACKGROUND_COLOR<0
#error The macro CONFIG_GRAPHICS_BACKGROUND_COLOR must be greater than or equal to 0.
#elif CONFIG_GRAPHICS_BACKGROUND_COLOR>0xFFFFFF
#error The macro CONFIG_GRAPHICS_BACKGROUND_COLOR must be less than or equal to 0xFFFFFF.
#endif /*CONFIG_GRAPHICS_BACKGROUND_COLOR*/

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

/* 
     * 绘制初始背景色。
     *
    if(CONFIG_FORCE_FILL_GRAPHICS_BACKGROUND)
    {
        UINT8 red_shift,red_width,green_shift,green_width,blue_shift,blue_width,color_size;
        env_mask_to_shift_width(params->graphics.red|params->graphics.green|params->graphics.blue|
            params->graphics.reserved,&red_shift,&red_width);
        ASSERT(red_width>0&&red_width<=32&&red_width%8==0);
        color_size=red_width>>3;
        env_mask_to_shift_width(params->graphics.red,&red_shift,&red_width);
        env_mask_to_shift_width(params->graphics.green,&green_shift,&green_width);
        env_mask_to_shift_width(params->graphics.blue,&blue_shift,&blue_width);
        env_graphics_pixel pixel={.pixel=0};
        pixel.pixel|=(env_scale_color(ENV_BACKGROUND_RED,red_width)<<red_shift);
        pixel.pixel|=(env_scale_color(ENV_BACKGROUND_GREEN,green_width)<<green_shift);
        pixel.pixel|=(env_scale_color(ENV_BACKGROUND_BLUE,blue_width)<<blue_shift);
        env_fill_color(pixel,color_size,params->graphics.scan_line,params->graphics.fb_base,
            0,0,params->graphics.hres,params->graphics.vres);
    }
    */

#endif /*__AOS_RESERVED_H__*/
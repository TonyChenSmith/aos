/**
 * 内核固件UEFI运行时服务。
 * @date 2026-04-19
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_FIREWARE_UEFI_H__
#define __AOS_KERNEL_FIREWARE_UEFI_H__

#include <support/const.h>

/**
 * EFI GUID。
 */
typedef struct _efi_guid
{
    uint32 data1;    /*数据1。*/
    uint16 data2;    /*数据2。*/
    uint16 data3;    /*数据3。*/
    uint8  data4[8]; /*数据4。*/
} efi_guid;

/**
 * EFI时间。
 */
typedef struct _efi_time
{
    uint16 year;       /*年。*/
    uint8  month;      /*月。*/
    uint8  day;        /*日。*/
    uint8  hour;       /*小时。*/
    uint8  minute;     /*分钟。*/
    uint8  second;     /*秒。*/
    uint8  pad1;       /*保留1。*/
    uint32 nanosecond; /*纳秒。*/
    int16  time_zone;  /*时区。*/
    uint8  daylight;   /*夏令时。*/
    uint8  pad2;       /*保留2。*/
} efi_time;

/**
 * EFI实时钟功能。
 */
typedef struct _efi_time_capabilities
{
    uint32 resolution;   /*分辨率。*/
    uint32 accuracy;     /*精度。*/
    bool   sets_to_zero; /*设置低于分辨率时间时是否会清零。*/
} efi_time_capabilities;

/**
 * EFI重置系统类型。
 */
typedef enum _efi_reset_type
{
    EFI_RESET_COLD,             /*冷重启。*/
    EFI_RESET_WARM,             /*暖重启。*/
    EFI_RESET_SHUTDOWN,         /*关机。*/
    EFI_RESET_PLATFORM_SPECIFIC /*平台定义。*/
} efi_reset_type;

/**
 * 检查是否为EFI错误状态码。
 */
#define efi_error(status) (status>=MAX_UINTN_BIT)

/**
 * EFI成功。
 */
#define EFI_SUCCESS ((uintn)0)

/**
 * EFI不支持。
 */
#define EFI_UNSUPPORTED ((uintn)(MAX_UINTN_BIT|3))

/**
 * EFI设备错误。
 */
#define EFI_DEVICE_ERROR ((uintn)(MAX_UINTN_BIT|7))

/**
 * 获得变量。
 * 
 * @param variable_name 变量名。
 * @param vendor_guid   提供者GUID。
 * @param attributes    属性。
 * @param data_size     数据大小。
 * @param data          数据。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
uintn efi_get_variable(char16* variable_name,efi_guid* vendor_guid,uint32* attributes,uintn* data_size,void* data);

/**
 * 获取下一个变量名。
 * 
 * @param variable_name_size 变量名大小。
 * @param variable_name      变量名。
 * @param vendor_guid        提供者GUID。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
uintn efi_get_next_variable_name(uintn* variable_name_size,char16* variable_name,efi_guid* vendor_guid);

/**
 * 设置变量。
 * 
 * @param variable_name 变量名。
 * @param vendor_guid   提供者GUID。
 * @param attributes    属性。
 * @param data_size     数据大小。
 * @param data          数据。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
uintn efi_set_variable(char16* variable_name,efi_guid* vendor_guid,uint32 attributes,uintn data_size,void* data);

/**
 * 获取时间。
 * 
 * @param time         EFI时间。
 * @param capabilities EFI实时钟功能。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
uintn efi_get_time(efi_time* time,efi_time_capabilities* capabilities);

/**
 * 设置时间。
 * 
 * @param time EFI时间。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
uintn efi_set_time(efi_time* time);

/**
 * 重置系统。
 * 
 * @param reset_type   重置类型。
 * @param reset_status 重置状态。
 * @param data_size    数据大小。
 * @param reset_data   重置数据。
 * 
 * @return 无法返回。
 */
void noreturn efi_reset_system(efi_reset_type reset_type,uintn reset_status,uintn data_size,void* reset_data);

/**
 * 获取高单调递增值。
 * 
 * @param high_count 高单调递增值。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
uintn efi_get_next_high_mono_count(uint32* high_count);

#endif /*__AOS_KERNEL_FIREWARE_UEFI_H__*/
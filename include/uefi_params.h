/**
 * 对UEFI固件下系统启动参数结构。在模块“aos.uefi”与模块“aos.kernel”间共享。
 * @date 2026-06-19
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_INCLUDE_UEFI_PARAMS_H__
#define __AOS_INCLUDE_UEFI_PARAMS_H__

/**
 * CPU特性。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _aos_cpu_features
{
    uint8 magic;    /*魔数。*/
    uint8 baseline; /*架构基线。*/
    uint8 vendor;   /*厂商。*/
    uint8 features; /*CPU特性。*/
} aos_cpu_features;

/**
 * CPU状态。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 * PAT设置为硬写值，按顺序分别是WB、WT、UC-、UC、WC、WP、UC-、UC。在引导阶段它们不会被使用，因此不定义宏常量。
 */
typedef struct _aos_cpu_state
{
    uint8 magic; /*魔数。*/
    uint8 state; /*设备状态。*/
    uint8 vmtrr; /*可变MTRR数目。*/
    uint8 apic;  /*APIC状态。*/
} aos_cpu_state;

/**
 * 图形信息。
 * 记录了在运行环境管理过程中提前探测的环境情况。
 */
typedef struct _aos_graphics_info
{
    uint32 hres;             /*水平分辨率。*/
    uint32 vres;             /*竖直分辨率。*/
    uintn  framebuffer_base; /*帧缓冲基址。*/
    uintn  framebuffer_size; /*帧缓冲大小。*/
    uint32 red_mask;         /*红色区掩码。*/
    uint32 green_mask;       /*绿色区掩码。*/
    uint32 blue_mask;        /*蓝色区掩码。*/
    uint32 reserved_mask;    /*保留区掩码。*/
    uint32 scan_line;        /*扫描线长度。*/
    uint32 pixel_clock;      /*像素时钟。*/
    uint32 horizontal_total; /*水平总行数。*/
    uint32 vertical_total;   /*竖直总列数。*/
    uint32 physical_width;   /*物理界面宽度。单位毫米。*/
    uint32 physical_height;  /*物理界面高度。单位毫米。*/
} aos_graphics_info;

/**
 * 线性区。这里使用了简单的单向链表结构用于传递信息。
 * 红黑树版本的在内核再实现。
 */
typedef struct _aos_boot_vma aos_boot_vma;

struct _aos_boot_vma
{
    aos_boot_vma* prev;  /*前一线性区。*/
    aos_boot_vma* next;  /*后一线性区。*/
    uintn         start; /*开始地址。*/
    uintn         end;   /*结束地址。*/
    uint64        flags; /*VMA标志。*/
    uint64        type;  /*VMA类型。*/
};

/**
 * AOS内存。
 */
typedef struct _aos_boot_memory aos_boot_memory;

struct _aos_boot_memory
{
    aos_boot_memory* prev;  /*前一内存。*/
    aos_boot_memory* next;  /*后一内存。*/
    uintn            start; /*起始地址。*/
    uintn            size;  /*内存大小。*/
    uint64           type;  /*内存类型。*/
};


/**
 * 内核内存信息。
 * 内核VMA管理应归属内核进程的线性区。
 * 固定VMA管理未分配所属权的已映射硬件资源，如固件。
 * 共享VMA管理在特权/用户级别共存的数据。
 */
typedef struct _aos_kernel_memory_info
{
    aos_boot_memory* memory_map;    /*内存图链表。*/
    aos_boot_vma*    kernel_vma;    /*内核VMA链表。*/
    uintn            page_table;    /*内核页表根地址。*/
    uintn            kernel_base;   /*内核程序线性基址。*/
    uintn            stack_base;    /*内核栈线性基址。*/
    uintn            kernel_entry;  /*内核入口偏移。*/
    uintn            firmware_base; /*固件映射线性基址。*/
    uintn            data_base;     /*内核数据基址。*/
    uint8*           pt_pool_map;   /*页表池位图映射。*/
    uintn            pt_pool_pages; /*页表池页面数。*/
} aos_kernel_memory_info;

/**
 * EFI设备路径。
 */
typedef struct _aos_efi_device_path
{
    uint8 type;      /*类型。*/
    uint8 sub_type;  /*子类型。*/
    uint8 length[2]; /*长度。*/
    uint8 data[];    /*数据数组。*/
} aos_efi_device_path;

/**
 * EFI内存描述符。
 */
typedef struct _aos_efi_memory_descriptor
{
    uint32 type;   /*类型。*/
    uint64 pstart; /*物理起始。*/
    uint64 vstart; /*线性起始。*/
    uint64 pages;  /*页数。*/
    uint64 attr;   /*属性。*/
} aos_efi_memory_descriptor;

/**
 * EFI表头。
 */
typedef struct _aos_efi_table_header{
    uint64 signature; /*签名。*/
    uint32 revision;  /*修订。*/
    uint32 size;      /*表头大小。*/
    uint32 crc32;     /*校验码。*/
    uint32 reserved;  /*保留。*/
} aos_efi_table_header;

/**
 * EFI GUID。
 */
typedef struct _aos_efi_guid
{
    uint32 data1;    /*数据1。*/
    uint16 data2;    /*数据2。*/
    uint16 data3;    /*数据3。*/
    uint8  data4[8]; /*数据4。*/
} aos_efi_guid;

/**
 * EFI时间。
 */
typedef struct _aos_efi_time
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
} aos_efi_time;

/**
 * EFI实时钟功能。
 */
typedef struct _aos_efi_time_capabilities
{
    uint32 resolution;   /*分辨率。*/
    uint32 accuracy;     /*精度。*/
    bool   sets_to_zero; /*设置低于分辨率时间时是否会清零。*/
} aos_efi_time_capabilities;

/**
 * EFI重置系统类型。
 */
typedef enum _aos_efi_reset_type
{
    AOS_EFI_RESET_COLD,             /*冷重启。*/
    AOS_EFI_RESET_WARM,             /*暖重启。*/
    AOS_EFI_RESET_SHUTDOWN,         /*关机。*/
    AOS_EFI_RESET_PLATFORM_SPECIFIC /*平台定义。*/
} aos_efi_reset_type;

/**
 * EFI胶囊头。
 */
typedef struct _aos_efi_capsule_header
{
    aos_efi_guid capsule_guid;       /*胶囊GUID。*/
    uint32       header_size;        /*头大小。*/
    uint32       flags;              /*标志。*/
    uint32       capsule_image_size; /*胶囊大小。*/
} aos_efi_capsule_header;

/**
 * 设置线性地址图。
 * 
 * @param memory_map_size    地址图大小。
 * @param descriptor_size    描述符大小。
 * @param descriptor_version 描述符版本。
 * @param virtual_map        线性地址图。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_set_virtual_address_map)(uintn memory_map_size,uintn descriptor_size,
    uint32 descriptor_version,aos_efi_memory_descriptor* virtual_map);

/**
 * 转换成线性地址。
 * 
 * @param debug_disposition 为地址提供类型信息。
 * @param address           被转换地址。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_convert_pointer)(uintn debug_disposition,void** address);

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
typedef uintn (AOS_EFIAPI *aos_efi_get_variable)(char16* variable_name,aos_efi_guid* vendor_guid,uint32* attributes,
    uintn* data_size,void* data);

/**
 * 获取下一个变量名。
 * 
 * @param variable_name_size 变量名大小。
 * @param variable_name      变量名。
 * @param vendor_guid        提供者GUID。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_get_next_variable_name)(uintn* variable_name_size,char16* variable_name,
    aos_efi_guid* vendor_guid);

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
typedef uintn (AOS_EFIAPI *aos_efi_set_variable)(char16* variable_name,aos_efi_guid* vendor_guid,uint32 attributes,
    uintn data_size,void* data);

/**
 * 获取时间。
 * 
 * @param time         EFI时间。
 * @param capabilities EFI实时钟功能。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_get_time)(aos_efi_time* time,aos_efi_time_capabilities* capabilities);

/**
 * 设置时间。
 * 
 * @param time EFI时间。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_set_time)(aos_efi_time* time);

/**
 * 获取唤醒时间。
 * 
 * @param enabled 是否启用。
 * @param pending 是否挂起。
 * @param time    EFI时间。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_get_wakeup_time)(bool* enabled,bool* pending,aos_efi_time* time);

/**
 * 设置唤醒时间。
 * 
 * @param enabled 是否启用。
 * @param time    EFI时间。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_set_wakeup_time)(bool enable,aos_efi_time* time);

/**
 * 重置系统。
 * 
 * @param reset_type   重置类型。
 * @param reset_status 重置状态。
 * @param data_size    数据大小。
 * @param reset_data   重置数据。
 * 
 * @return 无返回值。
 */
typedef void (AOS_EFIAPI *aos_efi_reset_system)(aos_efi_reset_type reset_type,uintn reset_status,uintn data_size,
    void* reset_data);

/**
 * 获取高单调递增值。
 * 
 * @param high_count 高单调递增值。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_get_next_high_mono_count)(uint32* high_count);

/**
 * 更新胶囊。
 * 
 * @param capsule_header_array 胶囊头数组。
 * @param capsule_count        胶囊数目。
 * @param scatter_gather_list  胶囊收集列表。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_update_capsule)(aos_efi_capsule_header** capsule_header_array,uintn capsule_count,
    uint64 scatter_gather_list);

/**
 * 查询胶囊功能。
 * 
 * @param capsule_header_array 胶囊头数组。
 * @param capsule_count        胶囊数目。
 * @param maximum_capsule_size 最大胶囊大小。
 * @param reset_type           重置类型。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_query_capsule_capabilities)(aos_efi_capsule_header** capsule_header_array,
    uintn capsule_count,uint64* maximum_capsule_size,aos_efi_reset_type* reset_type);

/**
 * 查询变量信息。
 * 
 * @param attributes                      属性。
 * @param maximum_variable_storage_size   最大变量存储空间大小。
 * @param remaining_variable_storage_size 剩余变量存储空间大小。
 * @param maximum_variable_size           最大变量大小。
 * 
 * @return EFI状态。具体看UEFI规范。
 */
typedef uintn (AOS_EFIAPI *aos_efi_query_variable_info)(uint32 attributes,uint64* maximum_variable_storage_size,
    uint64* remaining_variable_storage_size,uint64* maximum_variable_size);

/**
 * EFI运行时服务。用于定义交换。
 */
typedef struct _aos_efi_runtime_services
{
    aos_efi_table_header               hdr;                           /*表头。*/
    aos_efi_get_time                   get_time;                      /*获取时间。*/
    aos_efi_set_time                   set_time;                      /*设置时间。*/
    aos_efi_get_wakeup_time            get_wakeup_time;               /*获取唤醒时间。*/
    aos_efi_set_wakeup_time            set_wakeup_time;               /*设置唤醒时间。*/
    aos_efi_set_virtual_address_map    set_virtual_address_map;       /*设置线性地址图。*/
    aos_efi_convert_pointer            convert_pointer;               /*转换成线性地址。*/
    aos_efi_get_variable               get_variable;                  /*获得变量。*/
    aos_efi_get_next_variable_name     get_next_variable_name;        /*获取下一个变量名。*/
    aos_efi_set_variable               set_variable;                  /*设置变量。*/
    aos_efi_get_next_high_mono_count   get_next_high_monotonic_count; /*获取高单调递增值。*/
    aos_efi_reset_system               reset_system;                  /*重置系统。*/
    aos_efi_update_capsule             update_capsule;                /*更新胶囊。*/
    aos_efi_query_capsule_capabilities query_capsule_capabilities;    /*查询胶囊功能。*/
    aos_efi_query_variable_info        query_variable_info;           /*查询变量信息。*/
} aos_efi_runtime_services;

/**
 * EFI配置表。用于定义交换。
 */
typedef struct _aos_efi_configuration_table
{
    aos_efi_guid guid;  /*表标识ID。*/
    void*        table; /*表指针。*/
} aos_efi_configuration_table;

/**
 * EFI系统表。用于定义交换。
 */
typedef struct _aos_efi_system_table
{
    aos_efi_table_header         hdr;      /*表头。*/
    char16*                      vendor;   /*提供者。*/
    uint32                       revision; /*修订号。*/
    void*                        unused1;  /*未使用1。*/
    void*                        unused2;  /*未使用2。*/
    void*                        unused3;  /*未使用3。*/
    void*                        unused4;  /*未使用4。*/
    void*                        unused5;  /*未使用5。*/
    void*                        unused6;  /*未使用6。*/
    aos_efi_runtime_services*    runtime;  /*运行时服务表。*/
    void*                        unused7;  /*未使用7。*/
    uintn                        items;    /*配置表项数。*/
    aos_efi_configuration_table* tables;   /*配置表。*/
} aos_efi_system_table;

/**
 * 启动参数。
 * 记录了需要传递到内核的参数。
 */
typedef struct _aos_boot_params
{
    aos_cpu_features       features;        /*CPU特性。*/
    aos_cpu_state          state;           /*CPU状态。*/
    uintn                  acpi;            /*ACPI表。*/
    uintn                  smbios;          /*SMBIOS表。*/
    uintn                  cpu_count;       /*处理器数目。*/
    uintn                  enabled_count;   /*启用处理器数目。*/
    uint32*                cpus;            /*处理器数组。*/
    aos_graphics_info      graphics;        /*图形信息。*/
    aos_efi_device_path*   graphics_device; /*图形设备路径。*/
    aos_efi_device_path*   esp;             /*EFI系统分区设备路径。*/
    aos_efi_device_path*   asv;             /*AOS系统卷设备路径。*/
    aos_kernel_memory_info memory;          /*内核内存信息。*/
    aos_efi_system_table*  system_table;    /*UEFI系统表。*/
} aos_boot_params;

/**
 * 特性魔数。
 */
#define AOS_FEATURES_MAGIC 0xFE

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
#define AOS_FEATURES_NX 1

/**
 * 特性Page1GB标志位。
 */
#define AOS_FEATURES_PAGE1GB 2

/**
 * 特性LA57标志位。
 */
#define AOS_FEATURES_LA57 4

/**
 * 特性xAPIC标志位。
 */
#define AOS_FEATURES_XAPIC 8

/**
 * 特性x2APIC标志位。
 */
#define AOS_FEATURES_X2APIC 16

/**
 * 特性机器检查标志位。
 */
#define AOS_FEATURES_MC 32

/**
 * 状态魔数。
 */
#define AOS_STATE_MAGIC 0xAE

/**
 * 状态LA57标志位。
 */
#define AOS_STATE_LA57 1

/**
 * 状态固定MTRR标志位。
 */
#define AOS_STATE_FIXED_MTRR 2

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
 * 线性区内存缓存策略掩码。
 */
#define AOS_BOOT_VMA_CACHE_MASK 0xFF

/**
 * 线性区无缓存策略。
 */
#define AOS_BOOT_VMA_CACHE_UC 0

/**
 * 线性区写合并策略。
 */
#define AOS_BOOT_VMA_CACHE_WC 1

/**
 * 线性区写直通策略。
 */
#define AOS_BOOT_VMA_CACHE_WT 2

/**
 * 线性区写回策略。
 */
#define AOS_BOOT_VMA_CACHE_WB 3

/**
 * 线性区写保护策略。
 */
#define AOS_BOOT_VMA_CACHE_WP 4

/**
 * 线性区无缓存减策略。
 */
#define AOS_BOOT_VMA_CACHE_UCM 5

/**
 * 线性区可读标志。
 */
#define AOS_BOOT_VMA_READ 0x100

/**
 * 线性区可写标志。
 */
#define AOS_BOOT_VMA_WRITE 0x200

/**
 * 线性区可执行标志。
 */
#define AOS_BOOT_VMA_EXECUTE 0x400

/**
 * 线性区读写可执行掩码。
 */
#define AOS_BOOT_VMA_RWX_MASK (AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE)

/**
 * 线性区用户级标志。
 */
#define AOS_BOOT_VMA_USER 0x800

/**
 * 线性区全局标志。
 */
#define AOS_BOOT_VMA_GLOBAL 0x1000

/**
 * 线性区已分配标志。
 */
#define AOS_BOOT_VMA_ALLOCATED 0x2000

/**
 * 线性区大页标志。
 */
#define AOS_BOOT_VMA_HUGEPAGE 0x4000

/**
 * 线性区内核类型。
 */
#define AOS_BOOT_VMA_TYPE_KERNEL 0

/**
 * 线性区栈类型。
 */
#define AOS_BOOT_VMA_TYPE_STACK 1

/**
 * 线性区内存池类型。
 */
#define AOS_BOOT_VMA_TYPE_POOL 2

/**
 * 线性区固件类型。
 */
#define AOS_BOOT_VMA_TYPE_FIRMWARE 3

/**
 * 线性区帧类型。
 */
#define AOS_BOOT_VMA_TYPE_FRAME 4

/**
 * 线性区页表类型。
 */
#define AOS_BOOT_VMA_TYPE_PAGE_TABLE 5

/**
 * 线性区GDT类型。
 */
#define AOS_BOOT_VMA_TYPE_GDT 6

/**
 * 线性区IDT类型。
 */
#define AOS_BOOT_VMA_TYPE_IDT 7

/**
 * 内存自由内存类型。
 */
#define AOS_BOOT_MEMORY_FREE 0

/**
 * 内存引导池类型。
 */
#define AOS_BOOT_MEMORY_BOOTSTRAP_POOL 1

/**
 * 内存页表池类型。
 */
#define AOS_BOOT_MEMORY_PAGE_TABLE_POOL 2

/**
 * 内存内核池类型。
 */
#define AOS_BOOT_MEMORY_KERNEL_POOL 3

/**
 * 内存内核类型。
 */
#define AOS_BOOT_MEMORY_KERNEL 4

/**
 * 内存栈类型。
 */
#define AOS_BOOT_MEMORY_STACK 5

/**
 * 内存GDT类型。
 */
#define AOS_BOOT_MEMORY_GDT 6

/**
 * 内存IDT类型。
 */
#define AOS_BOOT_MEMORY_IDT 7

/**
 * 内存SMBIOS类型。
 */
#define AOS_BOOT_MEMORY_SMBIOS 8

/**
 * 内存固件类型。
 */
#define AOS_BOOT_MEMORY_FIRMWARE 9

/**
 * 内存ACPI表类型。
 */
#define AOS_BOOT_MEMORY_ACPI_TABLE 10

/**
 * 内存ACPI NVS类型。
 */
#define AOS_BOOT_MEMORY_ACPI_NVS 11

/**
 * 内存持久性内存类型。
 */
#define AOS_BOOT_MEMORY_PERSISTENT 12

/**
 * 内存保留类型。
 */
#define AOS_BOOT_MEMORY_RESERVED 13

/**
 * 内存最低1MB类型。
 */
#define AOS_BOOT_MEMORY_1MB 14

#endif /*__AOS_INCLUDE_UEFI_PARAMS_H__*/
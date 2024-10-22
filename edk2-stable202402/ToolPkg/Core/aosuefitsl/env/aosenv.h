/*
 * AOS Loader环境头文件。
 * 2024-05-29创建。 
 */
#ifndef __AOS_ENV_H__
#define __AOS_ENV_H__

#include "aosdefine.h"

#pragma pack(1)

/*AOS用段表*/
typedef struct
{
	IA32_SEGMENT_DESCRIPTOR start; /*第一个为空*/
	IA32_SEGMENT_DESCRIPTOR code32; /*32位ring0代码*/
	IA32_SEGMENT_DESCRIPTOR data32; /*32位ring0数据*/
	IA32_SEGMENT_DESCRIPTOR code64; /*64位ring0代码*/
	IA32_SEGMENT_DESCRIPTOR data64; /*64位ring0数据*/
	IA32_SEGMENT_DESCRIPTOR code64a; /*64位ring3代码*/
	IA32_SEGMENT_DESCRIPTOR data64a; /*64位ring3数据*/
	IA32_SEGMENT_DESCRIPTOR spare; /*对齐保留*/
} AOS_GDT_ENTRIES;

#pragma pack()

/*各个段地址*/
#define AOS_SEGMENT_START aos_offset_of(AOS_GDT_ENTRIES,start)
#define AOS_SEGMENT_CODE32 aos_offset_of(AOS_GDT_ENTRIES,code32)
#define AOS_SEGMENT_DATA32 aos_offset_of(AOS_GDT_ENTRIES,data32)
#define AOS_SEGMENT_CODE64 aos_offset_of(AOS_GDT_ENTRIES,code64)
#define AOS_SEGMENT_DATA64 aos_offset_of(AOS_GDT_ENTRIES,data64)
#define AOS_SEGMENT_CODE64A aos_offset_of(AOS_GDT_ENTRIES,code64a)
#define AOS_SEGMENT_DATA64A aos_offset_of(AOS_GDT_ENTRIES,data64a)
#define AOS_SEGMENT_SPARE aos_offset_of(AOS_GDT_ENTRIES,spare)

/*CPUID功能号*/
#define CPUID_STD_BASE_ID 0
#define CPU_INTEL_ID_EBX 0x756E6547
#define CPU_INTEL_ID_ECX 0X6C65746E
#define CPU_INTEL_ID_EDX 0x49656E69
#define CPU_AMD_ID_EBX 0x68747541
#define CPU_AMD_ID_ECX 0x444D4163
#define CPU_AMD_ID_EDX 0X69746E65

#define CPUID_STD_SEFID 0x7

#define CPUID_STD_ETE 0xB
#define CPUID_STD_V2ETE 0x1F
#define CPUID_ETE_LEVEL_INVALID 0

#define CPUID_STD_BASE_CPU 1

#define CPUID_EXT_MAX 0x80000000

#define CPUID_EXT_PCP 0x80000008

/*初始化内存*/
EFI_STATUS
EFIAPI
aos_init_memory(VOID);

/*申请内存页*/
VOID*
EFIAPI
aos_allocate_pages(
	IN UINTN pages
);

/*申请内存池*/
VOID*
EFIAPI
aos_allocate_pool(
	IN UINTN size
);

/*初始化CPU信息*/
EFI_STATUS
EFIAPI
aos_init_cpu_info(VOID);

/*创建配置*/
EFI_STATUS
EFIAPI
aos_create_config(VOID);

/*获取内存图*/
EFI_STATUS
EFIAPI
aos_set_memmap(
	OUT UINTN* map_key
);

/*
 * 设置系统表。
 */
EFI_STATUS
EFIAPI
aos_set_system_table(VOID);

#endif /*__AOS_ENV_H__*/
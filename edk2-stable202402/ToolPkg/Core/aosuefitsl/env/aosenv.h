/*
 * AOS Loader环境头文件。
 * 2024-05-29创建。 
 */
#ifndef __AOS_ENV_H__
#define __AOS_ENV_H__

#include "aosdefine.h"
#include "file/aosfile.h"

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

#define AOS_TSS_SPACE 128

/*CPUID功能号*/
#define CPUID_STD_BASE_ID 0
#define CPU_INTEL_ID_EBX 0x756E6547
#define CPU_INTEL_ID_ECX 0X6C65746E
#define CPU_INTEL_ID_EDX 0x49656E69
#define CPU_AMD_ID_EBX 0x68747541
#define CPU_AMD_ID_ECX 0x444D4163
#define CPU_AMD_ID_EDX 0X69746E65

#define CPUID_STD_FEAT 0x1

#define CPUID_STD_SEFID 0x7
#define CPUID_STD_SEFID_0 0

#define CPUID_STD_ETE 0xB
#define CPUID_STD_V2ETE 0x1F
#define CPUID_ETE_LEVEL_INVALID 0

#define CPUID_STD_BASE_CPU 1

#define CPUID_EXT_MAX 0x80000000

#define CPUID_EXT_FEAT 0x80000001

#define CPUID_EXT_PCP 0x80000008

/*MTRR*/
#define IA32_MTRRCAP 0x000000FE
#define IA32_MTRR_DEF_TYPE 0x000002FF
#define IA32_MTRR_FIX64K_00000 0x00000250
#define IA32_MTRR_FIX16K_80000 0x00000258
#define IA32_MTRR_FIX16K_A0000 0x00000259
#define IA32_MTRR_FIX4K_C0000 0x00000268
#define IA32_MTRR_FIX4K_C8000 0x00000269
#define IA32_MTRR_FIX4K_D0000 0x0000026A
#define IA32_MTRR_FIX4K_D8000 0x0000026B
#define IA32_MTRR_FIX4K_E0000 0x0000026C
#define IA32_MTRR_FIX4K_E8000 0x0000026D
#define IA32_MTRR_FIX4K_F0000 0x0000026E
#define IA32_MTRR_FIX4K_F8000 0x0000026F
#define IA32_MTRR_PHYSBASE_BASE 0x00000200
#define IA32_MTRR_PHYSMASK_BASE 0x00000201
#define VMTRR_BASE(n) (IA32_MTRR_PHYSBASE_BASE|(n<<1))
#define VMTRR_MASK(n) (IA32_MTRR_PHYSMASK_BASE|(n<<1))

#define MTRR_UC 0
#define MTRR_WC 1
#define MTRR_WT 4
#define MTRR_WP 5
#define MTRR_WB 6

/*MISC_ENABLE*/
#define IA32_MISC_ENABLE 0x000001A0

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

/*检查MSR信息，仅检查不记录*/
VOID
EFIAPI
aos_check_msr(VOID);

/*
 * 设置系统表。
 */
EFI_STATUS
EFIAPI
aos_set_system_table(VOID);

#endif /*__AOS_ENV_H__*/
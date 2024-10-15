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

/*创建配置*/
EFI_STATUS
EFIAPI
aos_create_config(VOID);

#endif /*__AOS_ENV_H__*/
/*
 * AOS内存类型定义。
 * @date 2024-11-14
 */
#ifndef __AOS_BASE_MEMTYPE_H__
#define __AOS_BASE_MEMTYPE_H__

/*内存类型枚举，物理内存和虚拟内存一致*/
typedef enum _aos_memory_type
{
	RESERVED,	 /*保留*/
	AVAILABLE,	 /*可用*/
	ACPI_TABLE,	 /*ACPI表*/
	ACPI_NVS,	 /*ACPI固件*/
	KERNEL_CODE, /*内核代码*/
	KERNEL_DATA, /*内核数据*/
	USER_CODE,	 /*用户代码*/
	USER_DATA,	 /*用户数据*/
	MMIO		 /*内存映射*/
} aos_memory_type;

#endif
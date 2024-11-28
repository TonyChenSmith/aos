/*
 * AOS内存类型。
 * @date 2024-11-14
 */
#ifndef __AOS_MEM_MTYPE_H__
#define __AOS_MEM_MTYPE_H__

/*内存类型*/
typedef enum _mtype
{
	RESERVED,	 /*保留*/
	AVAILABLE,	 /*可用*/
	ACPI_TABLE,	 /*ACPI表*/
	ACPI_NVS,	 /*ACPI固件*/
	FW_CODE,	 /*固件代码*/
	FW_DATA,	 /*固件数据*/
	KERNEL_CODE, /*内核代码*/
	KERNEL_DATA, /*内核数据*/
	USER_CODE,	 /*用户代码*/
	USER_DATA,	 /*用户数据*/
	MMIO,		 /*内存映射*/
	LOWEST,		 /*最低1MB区*/
	BOOT_DATA	 /*启动数据，仅构建表使用*/
} mtype;

/*
 * 物理内存和虚拟内存都使用这一套内存类型。
 *
 * 内存类型一般不能互相覆盖，除了以下关系：
 * 1.保留被重映射为内存映射。
 * 2.可用被映射为内核代码、内核数据、用户代码、用户数据。
 * 3.内核代码、内核数据、用户代码、用户数据取消映射改为可用。
 */

#endif /*__AOS_MEM_MTYPE_H__*/
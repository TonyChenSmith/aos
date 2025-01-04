/*
 * aos.boot模块配置。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_CONFIG_BOOT_H__
#define __AOS_BOOT_CONFIG_BOOT_H__

/*内存模块配置*/

/*初始物理内存位映射池结点。默认为26112，准许范围为1000-32576*/
#define BOOT_PHYSICAL_MEMORY_POOL 21760

/*初始页面位映射池结点。默认为255，准许范围为100-32576*/
#define BOOT_PAGE_TABLE_POOL 255

/*初始虚拟内存位映射池结点。默认为21760，准许范围为1000-32576*/
#define BOOT_VIRTUAL_MEMORY_POOL 21760

/*初始栈大小，为2MB页页数*/
#define BOOT_INITIAL_STACK 4 

#endif /*__AOS_BOOT_CONFIG_BOOT_H__*/
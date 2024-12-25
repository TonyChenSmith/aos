/*
 * aos.boot模块配置。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_CONFIG_BOOT_H__
#define __AOS_BOOT_CONFIG_BOOT_H__

/*内存模块配置*/

/*初始物理内存位映射池结点。默认为，准许范围为1000-26112*/
#define BOOT_PHYSICAL_MEMORY_POOL 26112

/*初始页面位映射池结点。默认为1023，准许范围为100-65535*/
#define BOOT_PAGE_TABLE_POOL 1023

/*初始虚拟内存位映射池结点。默认为，准许范围为1000-26112*/
#define BOOT_VIRTUAL_MEMORY_POOL 26112

#endif /*__AOS_BOOT_CONFIG_BOOT_H__*/
/*
 * aos.boot模块配置。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_CONFIG_H__
#define __AOS_BOOT_CONFIG_H__

/*内存模块配置*/

/*物理内存链表池大小。默认为4MB*/
#define BOOT_PM_POOL 0x400000

/*单个页面分配池可分配页数，大小支持从10到32576。默认为1023（补1页可有4MB）*/
#define BOOT_PT_POOL 1023

/*单个内存映射分配池可分配结点数，大小支持从64到32576。默认为26112（补1页可有1MB）*/
#define BOOT_MM_POOL 26112

#endif /*__AOS_BOOT_CONFIG_H__*/
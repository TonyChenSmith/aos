/*
 * aos.boot模块配置。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_CONFIG_H__
#define __AOS_BOOT_CONFIG_H__

/*页表池配置*/

/*页面分配池大小，PTP指代page table pool*/
#define BOOT_PTP_SIZE ((uintn)0x800000)

/*物理内存管理配置*/

/*物理内存列表预留长度*/
#define BOOT_PMML_SIZE ((uintn)1024)

#endif /*__AOS_BOOT_CONFIG_H__*/
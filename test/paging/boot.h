/*
 * aos.boot总头文件。在这里设置一些启动时内核的配置。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_BOOT_H__
#define __AOS_BOOT_BOOT_H__

#include "type.h"

/*页表池配置*/

/*页面分配池大小，PTP指代page table pool*/
#define BOOT_PTP_SIZE ((uintn)0x800000)

/*页面分配池页数*/
#define BOOT_PTP_PAGE (BOOT_PTP_SIZE>>12)

/*页面分配池位映射数组长度*/
#define BOOT_PTP_BITMAP (BOOT_PTP_PAGE>>6)

#endif /*__AOS_BOOT_BOOT_H__*/
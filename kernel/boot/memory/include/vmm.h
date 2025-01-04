/*
 * 虚拟内存管理函数。
 * @date 2024-12-25
 */
#ifndef __AOS_BOOT_MEMORY_VMM_H__
#define __AOS_BOOT_MEMORY_VMM_H__

#include "boot_bitmap.h"
#include "memory/buddy.h"
#include "memory/page_frame.h"
#include "memory/memory_defs.h"
#include "config/boot.h"
#include "param.h"
#include "pmm.h"

#if BOOT_VIRTUAL_MEMORY_POOL<1000||BOOT_VIRTUAL_MEMORY_POOL>32576
#error The value of BOOT_VIRTUAL_MEMORY_POOL is not within the supported range. Please reconfigure.
#endif

/*初始虚拟位映射池页数*/
#define BOOT_VMP_PAGE (bitmap_pool_page(BOOT_VIRTUAL_MEMORY_POOL,sizeof(virtual_page_frame)))

#endif

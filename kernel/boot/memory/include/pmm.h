/*
 * 物理内存管理函数。
 * @date 2024-11-14
 */
#ifndef __AOS_BOOT_MEMORY_PMM_H__
#define __AOS_BOOT_MEMORY_PMM_H__

#include "boot_bitmap.h"
#include "memory/buddy.h"
#include "memory/page_frame.h"
#include "memory/memory_defs.h"
#include "config/boot.h"
#include "param.h"

#if BOOT_PHYSICAL_MEMORY_POOL<1000||BOOT_PHYSICAL_MEMORY_POOL>32384
#error The value of BOOT_PHYSICAL_MEMORY_POOL is not within the supported range. Please reconfigure.
#endif

/*初始物理位映射池大小*/
#define BOOT_PMP_SIZE (bitmap_pool_page(BOOT_PHYSICAL_MEMORY_POOL,sizeof(physical_page_frame))<<12)

/*
 * 物理内存管理初始化。
 *
 * @param param 启动参数结构。
 * @param bbft	基础模块函数表。
 *
 * @return 无返回值。
 */
extern void boot_pmm_init(boot_params* restrict param,const boot_base_functions* bbft);

/*
 * 物理内存申请。其范围为闭区间。
 * 
 * @param mode	申请模式。
 * @param min	申请范围下限。
 * @param max	申请范围上限。
 * @param pages 申请页数。
 * @param type	申请类型。
 *
 * @return 对应内存基址，失败返回未定义。 
 */
extern uintn boot_pmm_alloc(const malloc_mode mode,const uintn min,const uintn max,const uintn pages,const memory_type type);

/*
 * 物理内存释放。
 * 
 * @param addr 需要释放区域内的地址。
 *
 * @return 无返回值。 
 */
extern void boot_pmm_free(const uintn addr);

#endif /*__AOS_BOOT_MEMORY_PMM_H__*/
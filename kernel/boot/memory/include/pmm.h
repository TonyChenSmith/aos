/*
 * 物理内存管理函数。
 * @date 2024-11-14
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BOOT_MEMORY_PMM_H__
#define __AOS_BOOT_MEMORY_PMM_H__

#include "bitmap.h"
#include "tree.h"
#include "memory/buddy.h"
#include "memory/page_frame.h"
#include "memory/memory_defs.h"
#include "config/boot.h"
#include "params.h"

#if BOOT_PHYSICAL_MEMORY_POOL<1000||BOOT_PHYSICAL_MEMORY_POOL>21760
#error The value of BOOT_PHYSICAL_MEMORY_POOL is not within the supported range. Please reconfigure.
#endif

/*初始物理位映射池页数*/
#define BOOT_PMP_PAGE bitmap_pool_page(BOOT_PHYSICAL_MEMORY_POOL,sizeof(physical_page_frame))

/*已使用里已分配树索引*/
#define PMMS_USED_ALLOCATED 0

/*已使用里已映射树索引*/
#define PMMS_USED_MAPPED 1

/*已使用里已保留树索引*/
#define PMMS_USED_RESERVED 2

/*系统分配池必要保留结点数。以最糟糕情况，从51分配到8需要44个结点。为避免上一次判断后发生最糟糕情况，需要预留两倍的结点为88，取整取90*/
#define PMMS_POOL_RESERVED 90

/*
 * 物理内存管理系统(PMMS)初始化。
 *
 * @param global 全局函数表。
 * @param params 启动核参数。
 * 
 * @return 无返回值。
 */
extern void boot_pmm_init(const boot_function_table* restrict global,const boot_params* restrict params);

/*
 * 物理内存申请。其范围为闭区间。
 * 
 * @param mode	申请模式。
 * @param min	申请范围下限。
 * @param max	申请范围上限。
 * @param pages 申请页数。
 * @param type	申请类型。
 * @param pid	所属处理器编号。
 *
 * @return 对应内存基址，失败返回未定义。 
 */
extern uintn boot_pmm_alloc(const malloc_mode mode,const uintn min,const uintn max,const uintn pages,const memory_type type,const uint32 pid);

/*
 * 物理内存释放。
 * 
 * @param addr 需要释放区域内的地址。
 *
 * @return 成功返回真，出错返回假。 
 */
extern bool boot_pmm_free(const uintn addr);

#endif /*__AOS_BOOT_MEMORY_PMM_H__*/
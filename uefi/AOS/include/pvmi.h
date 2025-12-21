/**
 * 模块页表与线性区管理所需的内部不公开内容。
 * @date 2025-08-24
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PVM_INTERNAL_H__
#define __AOS_UEFI_PVM_INTERNAL_H__

#include "mem.h"
#include "pvm.h"

#include <Guid/MemoryAttributesTable.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/TimerLib.h>

/**
 * 位图用掩码。
 */
CONST UINT8 PVM_BITMAP_MASK[]={BIT0,BIT1,BIT2,BIT3,BIT4,BIT5,BIT6,BIT7};

/**
 * 错误指针。
 */
UINT64* CONST PVM_ERROR=(UINT64*)0x101;

/**
 * 页表项使用位。
 */
#define PVM_PTE_P BIT0
#define PVM_PTE_RW BIT1
#define PVM_PTE_US BIT2
#define PVM_PTE_PWT BIT3
#define PVM_PTE_PCD BIT4
#define PVM_PTE_A BIT5
#define PVM_PTE_D BIT6
#define PVM_PTE_PAT BIT7
#define PVM_PTE_G BIT8
#define PVM_PTE_NX BIT63

/**
 * 页目录项使用位。
 */
#define PVM_PDE_PS BIT7
#define PVM_PDE_PAT BIT12

/**
 * 页面地址偏移掩码。
 */
#define PVM_PAGE_4K_OFFSET_MASK 0xFFF
#define PVM_PAGE_2M_OFFSET_MASK 0x1FFFFF
#define PVM_PAGE_1G_OFFSET_MASK 0x3FFFFFFF

/**
 * 页表项地址掩码。
 */
#define PVM_PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

/**
 * 页表项使用标志掩码。
 */
#define PVM_PTE_4K_FLAGS_MASK (PVM_PTE_RW|PVM_PTE_US|PVM_PTE_PWT|PVM_PTE_PCD|PVM_PTE_PAT|PVM_PTE_G|PVM_PTE_NX)
#define PVM_PTE_2M_FLAGS_MASK (PVM_PTE_RW|PVM_PTE_US|PVM_PTE_PWT|PVM_PTE_PCD|PVM_PTE_G|PVM_PDE_PAT|PVM_PTE_NX)
#define PVM_PTE_1G_FLAGS_MASK (PVM_PTE_RW|PVM_PTE_US|PVM_PTE_PWT|PVM_PTE_PCD|PVM_PTE_G|PVM_PDE_PAT|PVM_PTE_NX)

/**
 * 页目录项使用标志。
 */
#define PVM_PDE_FLAGS (PVM_PTE_P|PVM_PTE_RW|PVM_PTE_US)

/**
 * 物理地址保留掩码。
 */
#define PVM_PADDR_RESERVED_MASK 0xFFF0000000000000ULL

/**
 * 4级分页线性地址保留掩码。
 */
#define PVM_VADDR4_RESERVED_MASK 0xFFFF800000000000ULL

/**
 * 5级分页线性地址保留掩码。
 */
#define PVM_VADDR5_RESERVED_MASK 0xFF00000000000000ULL

/**
 * 随机魔数。这里是黄金比例常数。
 */
#define PVM_RANDOM_MAGIC 0x9E3779B9

/**
 * 代码段选择子。
 */
#define PVM_CODE_SEL 24

/**
 * 数据段选择子。
 */
#define PVM_DATA_SEL 32

#endif /*__AOS_UEFI_PVM_INTERNAL_H__*/
/* 
 * 模块“aos.uefi”页表与线性区管理功能。
 * 声明了仅在内部使用的的数据类型与宏，并在该文件内对所需配置做第二步检查。
 * @date 2025-08-24
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PVM_INTERNAL_H__
#define __AOS_UEFI_PVM_INTERNAL_H__

#include "pvm.h"
#include "mem.h"

#include <Library/BaseMemoryLib.h>

/* 
 * 位图用掩码。
 */
CONST UINT8 PVM_BITMAP_MASK[]={BIT0,BIT1,BIT2,BIT3,BIT4,BIT5,BIT6,BIT7};

/* 
 * 错误指针。
 */
UINT64* CONST PVM_ERROR=(UINT64*)0x101;

#endif /*__AOS_UEFI_PVM_INTERNAL_H__*/
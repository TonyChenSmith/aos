/**
 * 模块内存池管理所需的不公开内容。
 * @date 2025-06-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MEM_INTERNAL_H__
#define __AOS_UEFI_MEM_INTERNAL_H__

#include "mem.h"

#include <Library/BaseMemoryLib.h>

/**
 * TLSF块数据。
 * 前一块与当前块大小均为2的倍数，因此最低位0表示已分配，1表示空闲。
 */
typedef struct _mem_tlsf_block mem_tlsf_block;

struct _mem_tlsf_block
{
    UINTN           psize; /*前一块大小与分配状态。*/
    UINTN           csize; /*当前块大小与分配状态。*/
    mem_tlsf_block* prev;  /*双向链表前指针。*/
    mem_tlsf_block* next;  /*双向链表后指针。*/
    UINT64          magic; /*TLSF魔数。*/
};

/**
 * TLSF元数据。
 * 位图0代表无链表，1代表有链表。第一级每位分为了8个第二级。每一块最小大小为2倍块数据。
 * 当前应该为24字节空闲空间，也就是总共64字节大小。因此0级范围为[64,128)，下一级以此类推，直到31级为[2^37,2^38)。
 * 31级的大小已经足够大，在当前阶段可以设定为无限，即最高级仅保留最低级不再分级。
 * 在元数据内有空闲链表数组与已分配数组两个部分，用于跟踪块数据分配情况。链表为双向链表。
 */
typedef struct _mem_tlsf_meta
{
    UINT32          magic;          /*TLSF魔数。*/
    UINT32          fl_bitmap;      /*第一级位图。*/
    UINT8           sl_bitmap[32];  /*第二级位图。*/
    mem_tlsf_block* free[32][8][2]; /*空闲块链表。*/
    mem_tlsf_block* alloc[2];       /*已分配块链表。*/
} mem_tlsf_meta;

/**
 * TLSF空闲块状态。
 */
#define MEM_TLSF_BLOCK_FREE 1

/**
 * TLSF已分配块状态。
 */
#define MEM_TLSF_BLOCK_ALLOC 0

/**
 * TLSF元数据魔数。
 */
#define MEM_TLSF_MAGIC_META SIGNATURE_32('T','L','S','F')

/**
 * TLSF空闲魔数。
 */
#define MEM_TLSF_MAGIC_FREE SIGNATURE_64('T','L','S','F','F','R','E','E')

/**
 * TLSF已分配魔数。
 */
#define MEM_TLSF_MAGIC_ALLOC SIGNATURE_64('T','L','S','F','A','L','L','O')

/**
 * 设置TLSF块数据状态。
 */
#define mem_tlsf_set_state(size,state) (((size)&(MAX_UINTN^BIT0))|(state&BIT0))

/**
 * 获取TLSF块数据状态。
 */
#define mem_tlsf_get_state(size) ((size)&BIT0)

/**
 * 设置TLSF块数据状态。
 */
#define mem_tlsf_get_size(size) ((size)&(MAX_UINTN^BIT0))

/**
 * 设置TLSF第一级位图。
 */
#define mem_tlsf_set_fl(bitmap,fl) ((bitmap)|(1U<<(fl)))

/**
 * 清空TLSF第一级位图。
 */
#define mem_tlsf_clear_fl(bitmap,fl) ((bitmap)&(~(1U<<(fl))))

/**
 * 获取TLSF第一级位图。
 */
#define mem_tlsf_get_fl(bitmap,fl) ((bitmap)&(1U<<(fl)))

/**
 * 设置TLSF第二级位图。
 */
#define mem_tlsf_set_sl(bitmap,sl) ((bitmap)|(1U<<(sl)))

/**
 * 清空TLSF第二级位图。
 */
#define mem_tlsf_clear_sl(bitmap,sl) ((bitmap)&(~(1U<<(sl))))

/**
 * 获取TLSF第二级位图。
 */
#define mem_tlsf_get_sl(bitmap,sl) ((bitmap)&(1U<<(sl)))

/**
 * 对内存大小进行对齐。
 */
#define mem_align_size(size) (size<24?24:ALIGN_VALUE(size,8))

/**
 * 第二轮检查。
 * 主要根据对使用宏的数据类型的具体定义确定检查范围。
 */

/**
 * 总页数检查。
 * 这里主要是避免过大，虚拟机默认低4GB映射了2GB，合计使用超过1GB的内存池没有意义。
 */
#if (CONFIG_BOOTSTRAP_POOL+CONFIG_KERNEL_POOL+CONFIG_PAGE_TABLE_POOL)>EFI_SIZE_TO_PAGES(SIZE_1GB)
#error The required memory is too large.
#endif

#endif /*__AOS_UEFI_MEM_INTERNAL_H__*/
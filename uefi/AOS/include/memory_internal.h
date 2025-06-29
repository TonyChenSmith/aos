/* 
 * 模块“aos.uefi”内存池管理内部声明。
 * 声明了仅用于内存池管理的数据类型与宏，并在该文件内对所需配置做第二步检查。
 * @date 2025-06-05
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_MEMORY_INTERNAL_H__
#define __AOS_UEFI_MEMORY_INTERNAL_H__

#include "memory.h"

#include <Library/BaseMemoryLib.h>

/* 
 * 位图类型。
 */
typedef struct _memory_bitmap memory_bitmap;

/* 
 * TLSF块数据类型。
 */
typedef struct _memory_tlsf_block memory_tlsf_block;

/* 
 * TLSF元数据类型。
 */
typedef struct _memory_tlsf_meta memory_tlsf_meta;

/* 
 * 位图。
 * 位图0代表未分配，1代表已分配。结构中部分量仅用于加速计算。
 */
struct _memory_bitmap
{
    UINT32 offset;   /*位图后偏移，包含该位图大小。*/
    UINT16 free;     /*位图未分配页数。*/
    UINT16 size;     /*位图管理页数。*/
    UINT64 bitmap[]; /*位图数组。*/
};

/* 
 * TLSF块数据。
 * 前一块与当前块大小均为2的倍数，因此最低位0表示已分配，1表示空闲。
 */
struct _memory_tlsf_block
{
    UINTN              psize; /*前一块大小与分配状态。*/
    UINTN              csize; /*当前块大小与分配状态。*/
    memory_tlsf_block* prev;  /*双向链表前指针。*/
    memory_tlsf_block* next;  /*双向链表后指针。*/
    UINT64             magic; /*TLSF魔数。*/
};

/* 
 * TLSF元数据。
 * 位图0代表无链表，1代表有链表。第一级每位分为了8个第二级。每一块最小大小为2倍块数据。
 * 当前应该为24字节空闲空间，也就是总共64字节大小。因此0级范围为[64,128)，下一级以此类推，直到31级为[2^37,2^38)。
 * 31级的大小已经足够大，在初始化阶段可以认为无限。
 * 在元数据内有空闲链表数组与已分配数组两个部分，用于跟踪块数据分配情况。链表以一个长度为2的数组表示头节点与尾结点。
 */
struct _memory_tlsf_meta
{
    UINT32             magic;          /*TLSF魔数。*/
    UINT32             fl_bitmap;      /*第一级位图。*/
    UINT8              sl_bitmap[32];  /*第二级位图。*/
    memory_tlsf_block* free[32][8][2]; /*空闲块链表。*/
    memory_tlsf_block* alloc[2];       /*已分配块链表。*/
};

/* 
 * TLSF空闲块状态。
 */
#define MEMORY_TLSF_BLOCK_FREE 1

/* 
 * TLSF已分配块状态。
 */
#define MEMORY_TLSF_BLOCK_ALLOC 0

/* 
 * TLSF元数据魔数。
 */
#define MEMORY_TLSF_MAGIC_META SIGNATURE_32('T','L','S','F')

/* 
 * TLSF空闲魔数。
 */
#define MEMORY_TLSF_MAGIC_FREE SIGNATURE_64('T','L','S','F','F','R','E','E')

/* 
 * TLSF已分配魔数。
 */
#define MEMORY_TLSF_MAGIC_ALLOC SIGNATURE_64('T','L','S','F','A','L','L','O')

/* 
 * 获取内存位图数组长度。
 */
#define memory_bitmap_length(pages) (pages/64+((pages%64)?1:0))

/* 
 * 获取内存位图大小。
 */
#define memory_bitmap_size(pages) (sizeof(memory_bitmap)+memory_bitmap_length(pages)*sizeof(UINT64))

/* 
 * 获取内存位图后偏移。
 */
#define memory_bitmap_offset(pages) ((UINT32)memory_bitmap_size(pages))

/* 
 * 获取初始最大空闲块偏移。
 */
#define memory_init_free_offset(pages) (memory_bitmap_size(pages)+sizeof(memory_tlsf_meta))

/* 
 * 获取初始最大空闲块大小。
 */
#define memory_init_free_size(pool,preallocated) \
    (EFI_PAGES_TO_SIZE(preallocated)-memory_init_free_offset(pool)-sizeof(memory_tlsf_block))

/* 
 * 设置TLSF块数据状态。
 */
#define memory_tlsf_set_state(size,state) (((size)&(MAX_UINTN^1))|(state&1))

/* 
 * 获取TLSF块数据状态。
 */
#define memory_tlsf_get_state(size) ((size)&1)

/* 
 * 设置TLSF块数据状态。
 */
#define memory_tlsf_get_size(size) ((size)&(MAX_UINTN^1))

/* 
 * 设置TLSF第一级位图。
 */
#define memory_tlsf_set_fl(bitmap,fl) ((bitmap)|(1U<<(fl)))

/* 
 * 清空TLSF第一级位图。
 */
#define memory_tlsf_clear_fl(bitmap,fl) ((bitmap)&(~(1U<<(fl))))

/* 
 * 获取TLSF第一级位图。
 */
#define memory_tlsf_get_fl(bitmap,fl) ((bitmap)&(1U<<(fl)))

/* 
 * 设置TLSF第二级位图。
 */
#define memory_tlsf_set_sl(bitmap,sl) ((bitmap)|(1U<<(sl)))

/* 
 * 清空TLSF第二级位图。
 */
#define memory_tlsf_clear_sl(bitmap,sl) ((bitmap)&(~(1U<<(sl))))

/* 
 * 获取TLSF第二级位图。
 */
#define memory_tlsf_get_sl(bitmap,sl) ((bitmap)&(1U<<(sl)))

/* 
 * 第二轮检查。
 * 主要根据对使用宏的数据类型的具体定义确定检查范围。
 */

/* 
 * CONFIG_MEMORY_POOL_PAGES检查。
 * 这里主要是避免过大，虚拟机默认低4GB映射了2GB，作为初始化使用超过1GB的池没有意义。
 */
#if CONFIG_MEMORY_POOL_PAGES>EFI_SIZE_TO_PAGES(SIZE_1GB)
#error The macro CONFIG_MEMORY_POOL_PAGES is too large.
#endif /*CONFIG_MEMORY_POOL_PAGES*/

/* 
 * CONFIG_MEMORY_PREALLOCATED_PAGES检查。
 * 这里主要是保证预取页数足够包含必要结构。
 */
#if CONFIG_MEMORY_PREALLOCATED_PAGES<EFI_SIZE_TO_PAGES(8+memory_bitmap_length(CONFIG_MEMORY_POOL_PAGES)*8+4152+64)
#error The macro CONFIG_MEMORY_PREALLOCATED_PAGES is too small.
#endif /*CONFIG_MEMORY_PREALLOCATED_PAGES*/

#endif /*__AOS_UEFI_MEMORY_INTERNAL_H__*/
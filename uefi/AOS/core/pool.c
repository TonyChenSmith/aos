/**
 * 模块引导内存池管理。
 * @date 2026-06-27
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "config.h"
#include "panic.h"

/**
 * TLSF块数据。
 * 前一块与当前块大小均为2的倍数，因此最低位0表示已分配，1表示空闲。
 */
typedef struct _pool_tlsf_block pool_tlsf_block;

struct _pool_tlsf_block
{
    UINTN            psize; /*前一块大小与分配状态。*/
    UINTN            csize; /*当前块大小与分配状态。*/
    pool_tlsf_block* prev;  /*双向链表前指针。*/
    pool_tlsf_block* next;  /*双向链表后指针。*/
};

/**
 * TLSF元数据。
 * 位图0代表无链表，1代表有链表。第一级每位分为了8个第二级。每一块最小大小为2倍块数据。
 * 当前应该为32字节空闲空间，也就是总共64字节大小。因此0级范围为[64,128)，下一级以此类推，直到15级为[2^21,2^22)。
 * 16级的大小已经足够大，在当前阶段可以设定为无限，即最高级仅保留最低级不再分级。
 * 在元数据内有空闲链表数组与已分配数组两个部分，用于跟踪块数据分配情况。链表为双向链表。
 */
typedef struct _pool_tlsf_meta
{
    UINT16           flmap;          /*第一级位图。*/
    UINT8            slmap[16];      /*第二级位图。*/
    pool_tlsf_block* free[16][8][2]; /*空闲块链表。*/
    pool_tlsf_block* alloc[2];       /*已分配块链表。*/
} pool_tlsf_meta;

/**
 * 设置TLSF块数据状态。
 */
#define pool_tlsf_set_state(size,state) (((size)&(MAX_UINTN^BIT0))|(state&BIT0))

/**
 * 获取TLSF块数据状态。
 */
#define pool_tlsf_get_state(size) ((size)&BIT0)

/**
 * 设置TLSF块数据状态。
 */
#define pool_tlsf_get_size(size) ((size)&(MAX_UINTN^BIT0))

/**
 * 设置TLSF第一级位图。
 */
#define pool_tlsf_set_fl(bitmap,fl) ((bitmap)|(1U<<(fl)))

/**
 * 清空TLSF第一级位图。
 */
#define pool_tlsf_clear_fl(bitmap,fl) ((bitmap)&(~(1U<<(fl))))

/**
 * 获取TLSF第一级位图。
 */
#define pool_tlsf_get_fl(bitmap,fl) ((bitmap)&(1U<<(fl)))

/**
 * 设置TLSF第二级位图。
 */
#define pool_tlsf_set_sl(bitmap,sl) ((bitmap)|(1U<<(sl)))

/**
 * 清空TLSF第二级位图。
 */
#define pool_tlsf_clear_sl(bitmap,sl) ((bitmap)&(~(1U<<(sl))))

/**
 * 获取TLSF第二级位图。
 */
#define pool_tlsf_get_sl(bitmap,sl) ((bitmap)&(1U<<(sl)))

/**
 * 对内存大小进行对齐。
 */
#define pool_align_size(size) (size<32?32:ALIGN_VALUE(size,8))

/**
 * TLSF空闲块状态。
 */
#define POOL_TLSF_BLOCK_FREE 1

/**
 * TLSF已分配块状态。
 */
#define POOL_TLSF_BLOCK_ALLOC 0

/**
 * 获取大小对应第一级索引。
 * 
 * @param size 块大小。
 * 
 * @return 返回对应第一级索引。
 */
STATIC UINT8 EFIAPI pool_fl_index(IN UINTN size)
{
    size=pool_tlsf_get_size(size);
    if(size<64)
    {
        return 0;
    }
    UINT8 fl=(UINT8)HighBitSet64(size)-6;
    if(fl>=15)
    {
        return 15;
    }
    else
    {
        return fl;
    }
}

/**
 * 获取大小对应第二级索引。
 * 
 * @param size 块大小。
 * @param fl   第一级索引。
 * 
 * @return 返回对应第二级索引。
 */
STATIC UINT8 EFIAPI pool_sl_index(IN UINTN size,IN UINT8 fl)
{
    size=pool_tlsf_get_size(size);
    if(size<64||fl==15)
    {
        return 0;
    }
    return (size>>(fl+3))&0x7;
}

/**
 * 将结点添加到双向链表内。
 * 
 * @param list 双向链表。
 * @param node 结点。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pool_list_add(IN pool_tlsf_block** list,IN pool_tlsf_block* node)
{
    node->prev=NULL;
    node->next=list[0];
    if(list[0]==NULL)
    {
        list[1]=node;
    }
    else
    {
        list[0]->prev=node;
    }
    list[0]=node;
}

/**
 * 将结点从双向链表内删除。
 * 
 * @param list 双向链表。
 * @param node 结点。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pool_list_remove(IN pool_tlsf_block** list,IN pool_tlsf_block* node)
{
    if(node->prev!=NULL)
    {
        node->prev->next=node->next;
    }
    else
    {
        list[0]=node->next;
    }
    if(node->next!=NULL)
    {
        node->next->prev=node->prev;
    }
    else
    {
        list[1]=node->prev;
    }
    node->prev=NULL;
    node->next=NULL;
}

/**
 * 引导内存池。
 */
pool_tlsf_meta* pool=NULL;

/**
 * 初始化引导内存池。
 * 
 * @return 申请到的引导内存池基址。
 */
UINTN EFIAPI init_bootstrap_pool(VOID)
{
    if((CONFIG_BOOTSTRAP_POOL*EFI_PAGE_SIZE)<(sizeof(pool_tlsf_meta)+2*sizeof(pool_tlsf_block)+32))
    {
        panic(EFI_INVALID_PARAMETER,"Failed to initialize bootstrap pool.");
    }

    EFI_PHYSICAL_ADDRESS bpool=SIZE_2GB;
    panic_if_error(gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,CONFIG_BOOTSTRAP_POOL,&bpool),
        "Failed to allocate bootstrap pool.");

    pool_tlsf_meta* meta=(pool_tlsf_meta*)bpool;
    ZeroMem(meta,sizeof(pool_tlsf_meta));

    pool_tlsf_block* block=(pool_tlsf_block*)((UINTN)bpool+sizeof(pool_tlsf_meta));
    block->psize=pool_tlsf_set_state(0,POOL_TLSF_BLOCK_ALLOC);
    block->csize=pool_tlsf_set_state((CONFIG_BOOTSTRAP_POOL*EFI_PAGE_SIZE)-sizeof(pool_tlsf_meta)
        -sizeof(pool_tlsf_block),POOL_TLSF_BLOCK_FREE);

    UINT8 fl=pool_fl_index(block->csize);
    UINT8 sl=pool_sl_index(block->csize,fl);
    meta->flmap=pool_tlsf_set_fl(meta->flmap,fl);
    meta->slmap[fl]=pool_tlsf_set_sl(meta->slmap[fl],sl);
    pool_list_add(meta->free[fl][sl],block);

    pool_tlsf_block* sentinel=(pool_tlsf_block*)((UINTN)bpool+(CONFIG_BOOTSTRAP_POOL*EFI_PAGE_SIZE)
        -sizeof(pool_tlsf_block));
    sentinel->psize=block->csize;
    sentinel->csize=pool_tlsf_set_state(sizeof(pool_tlsf_block),POOL_TLSF_BLOCK_ALLOC);
    pool_list_add(meta->alloc,sentinel);

    pool=meta;
    return (UINTN)bpool;
}

/**
 * 将空闲结点从内存池空闲链表中删除，并按情况进行分配。
 * 
 * @param size 申请所需大小。
 * @param node TLSF块结点。
 * @param fl   TLSF块结点链表所在第一级。
 * @param sl   TLSF块结点链表所在第二级。
 * 
 * @return 数据区基址。
 */
STATIC VOID* EFIAPI pool_block_alloc(IN UINTN size,IN pool_tlsf_block* node,IN UINT8 fl,IN UINT8 sl)
{
    pool_list_remove(pool->free[fl][sl],node);

    if(pool->free[fl][sl][0]==NULL)
    {
        pool->slmap[fl]=pool_tlsf_clear_sl(pool->slmap[fl],sl);
        if(pool->slmap[fl]==0)
        {
            pool->flmap=pool_tlsf_clear_fl(pool->flmap,fl);
        }
    }

    pool_tlsf_block* next=(pool_tlsf_block*)((UINTN)node+pool_tlsf_get_size(node->csize));
    if(pool_tlsf_get_size(node->csize)-size>=64)
    {
        pool_tlsf_block* free=(pool_tlsf_block*)((UINTN)node+size);
        free->psize=pool_tlsf_set_state(size,POOL_TLSF_BLOCK_ALLOC);
        free->csize=pool_tlsf_set_state(pool_tlsf_get_size(node->csize)-size,POOL_TLSF_BLOCK_FREE);
        next->psize=free->csize;
        UINT8 free_fl=pool_fl_index(free->csize);
        UINT8 free_sl=pool_sl_index(free->csize,free_fl);

        pool->flmap=pool_tlsf_set_fl(pool->flmap,free_fl);
        pool->slmap[free_fl]=pool_tlsf_set_sl(pool->slmap[free_fl],free_sl);
        pool_list_add(pool->free[free_fl][free_sl],free);

        node->csize=free->psize;
    }
    else
    {
        node->csize=pool_tlsf_set_state(node->csize,POOL_TLSF_BLOCK_ALLOC);
        next->psize=node->csize;
    }

    pool_list_add(pool->alloc,node);
    return (VOID*)((UINTN)node+sizeof(pool_tlsf_block));
}

/**
 * 在引导内存池内申请一块内存。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI pool_alloc(IN UINTN size)
{
    if(pool==NULL||size==0)
    {
        return NULL;
    }

    size=pool_align_size(size)+sizeof(pool_tlsf_block);
    UINT8 fl=pool_fl_index(size);
    UINT8 sl=pool_sl_index(size,fl);
    if(size>(1ULL<<(fl+6))+(sl<<(fl+3))&&fl<15)
    {
        if(sl<7)
        {
            sl++;
        }
        else
        {
            fl++;
            sl=0;
        }
    }

    if(fl==15)
    {
        pool_tlsf_block* node=pool->free[15][0][0];
        while(node!=NULL)
        {
            if(size<=pool_tlsf_get_size(node->csize))
            {
                return pool_block_alloc(size,node,15,0);
            }
            node=node->next;
        }
        return NULL;
    }

    INTN find_fl=LowBitSet32(pool->flmap&(MAX_UINT16<<fl));
    if(find_fl<0)
    {
        return NULL;
    }
    else if(find_fl==fl)
    {
        INTN find_sl=LowBitSet32(pool->slmap[fl]&(MAX_UINT8<<sl));
        if(find_sl<0)
        {
            find_fl=LowBitSet32(pool->flmap&(MAX_UINT16<<(fl+1)));
            if(find_fl<0)
            {
                return NULL;
            }
            else
            {
                find_sl=LowBitSet32(pool->slmap[find_fl]);
            }
        }
        return pool_block_alloc(size,pool->free[find_fl][find_sl][0],(UINT8)find_fl,(UINT8)find_sl);
    }
    else
    {
        INTN find_sl=LowBitSet32(pool->slmap[find_fl]);
        return pool_block_alloc(size,pool->free[find_fl][find_sl][0],(UINT8)find_fl,(UINT8)find_sl);
    }
}

/**
 * 向引导内存池释放一块内存。
 * 
 * @param pointer 被释放指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI pool_free(IN VOID* pointer)
{
    if(pool==NULL||pointer==NULL)
    {
        return;
    }

    pool_tlsf_block* node=(pool_tlsf_block*)((UINTN)pointer-sizeof(pool_tlsf_block));
    if(pool_tlsf_get_size(node->csize)<64||pool_tlsf_get_size(node->csize)%8!=0)
    {
        return;
    }

    pool_list_remove(pool->alloc,node);
    pool_tlsf_block* next=(pool_tlsf_block*)((UINTN)node+pool_tlsf_get_size(node->csize));
    if(pool_tlsf_get_state(next->csize))
    {
        UINT8 next_fl=pool_fl_index(next->csize);
        UINT8 next_sl=pool_sl_index(next->csize,next_fl);
        pool_list_remove(pool->free[next_fl][next_sl],next);
        if(pool->free[next_fl][next_sl][0]==NULL)
        {
            pool->slmap[next_fl]=pool_tlsf_clear_sl(pool->slmap[next_fl],next_sl);
            if(pool->slmap[next_fl]==0)
            {
                pool->flmap=pool_tlsf_clear_fl(pool->flmap,next_fl);
            }
        }
        node->csize=pool_tlsf_set_state(pool_tlsf_get_size(node->csize)+pool_tlsf_get_size(next->csize),
            POOL_TLSF_BLOCK_FREE);
        next=(pool_tlsf_block*)((UINTN)next+pool_tlsf_get_size(next->csize));
        next->psize=node->csize;
    }
    else
    {
        node->csize=pool_tlsf_set_state(node->csize,POOL_TLSF_BLOCK_FREE);
        next->psize=node->csize;
    }

    if(pool_tlsf_get_state(node->psize))
    {
        pool_tlsf_block* prev=(pool_tlsf_block*)((UINTN)node-pool_tlsf_get_size(node->psize));
        UINT8 prev_fl=pool_fl_index(prev->csize);
        UINT8 prev_sl=pool_sl_index(prev->csize,prev_fl);
        pool_list_remove(pool->free[prev_fl][prev_sl],prev);
        if(pool->free[prev_fl][prev_sl][0]==NULL)
        {
            pool->slmap[prev_fl]=pool_tlsf_clear_sl(pool->slmap[prev_fl],prev_sl);
            if(pool->slmap[prev_fl]==0)
            {
                pool->flmap=pool_tlsf_clear_fl(pool->flmap,prev_fl);
            }
        }
        prev->csize=pool_tlsf_set_state(pool_tlsf_get_size(node->csize)+pool_tlsf_get_size(prev->csize),
            POOL_TLSF_BLOCK_FREE);
        node=prev;
        next->psize=node->csize;
    }

    UINT8 node_fl=pool_fl_index(node->csize);
    UINT8 node_sl=pool_sl_index(node->csize,node_fl);
    pool->flmap=pool_tlsf_set_fl(pool->flmap,node_fl);
    pool->slmap[node_fl]=pool_tlsf_set_sl(pool->slmap[node_fl],node_sl);
    pool_list_add(pool->free[node_fl][node_sl],node);
}
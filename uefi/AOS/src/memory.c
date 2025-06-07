/*
 * 模块“aos.uefi”内存管理。
 * 实现了与模块内存相关的函数，以及便于调试该部分功能的函数。
 * @date 2025-06-06
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "memory.h"
#include "memory_internal.h"

/*
 * 获取大小对应第一级索引。
 * 
 * @param size 块大小。
 * 
 * @return 返回对应第一级索引。
 */
STATIC UINT8 EFIAPI inline memory_fl_index(UINTN size)
{
    size=memory_tlsf_get_size(size);
    if(size<64)
    {
        return 0;
    }
    UINT8 fl=(UINT8)HighBitSet64(size)-6;
    if(fl>=31)
    {
        return 31;
    }
    else
    {
        return fl;
    }
}

/*
 * 获取大小对应第二级索引。
 * 
 * @param size 块大小。
 * @param fl   第一级索引。
 * 
 * @return 返回对应第二级索引。
 */
STATIC UINT8 EFIAPI inline memory_sl_index(UINTN size,UINT8 fl)
{
    size=memory_tlsf_get_size(size);
    if(size<64||fl==31)
    {
        return 0;
    }
    return (size>>(fl+3))&0x7;
}


/*
 * 将结点添加到双向链表内。
 * 
 * @param list 双向链表。
 * @param node 结点。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI memory_list_add(memory_tlsf_block** list,memory_tlsf_block* node)
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

/*
 * 将结点从双向链表内删除。
 * 
 * @param list 双向链表。
 * @param node 结点。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI memory_list_remove(memory_tlsf_block** list,memory_tlsf_block* node)
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

/*
 * 初始化内存池。包括申请内存页，构造位图、元数据和块数据。
 * 
 * @param bitmap 位图地址。
 * @param meta   TLSF元数据地址。
 * 
 * @return 正常返回EFI_SUCCESS。异常返回对应错误码。
 */
STATIC EFI_STATUS EFIAPI memory_init_pool(OUT memory_bitmap** bitmap,OUT memory_tlsf_meta** meta)
{
    EFI_STATUS status;

    /*分配内存池*/
    EFI_PHYSICAL_ADDRESS base;
    status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,CONFIG_MEMORY_POOL_PAGES,&base);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.memory] Failed to allocate memory pool of %llu pages.\n",
            (UINT64)CONFIG_MEMORY_POOL_PAGES));
        return status;
    }

    /*构造位图数据*/
    memory_bitmap* cbitmap=(memory_bitmap*)base;
    cbitmap->size=CONFIG_MEMORY_POOL_PAGES;
    cbitmap->free=CONFIG_MEMORY_POOL_PAGES-CONFIG_MEMORY_PREALLOCATED_PAGES;
    cbitmap->offset=memory_bitmap_offset(CONFIG_MEMORY_POOL_PAGES);
    ZeroMem(cbitmap->bitmap,memory_bitmap_length(CONFIG_MEMORY_POOL_PAGES)*sizeof(UINT64));

    /*置预分配位*/
    UINT64 bits=CONFIG_MEMORY_PREALLOCATED_PAGES;
    UINTN index=0;
    while(bits>64)
    {
        cbitmap->bitmap[index]=MAX_UINT64;
        index++;
        bits=bits-64;
    }
    if(bits>0)
    {
        cbitmap->bitmap[index]|=(1ULL<<bits)-1;
    }

    /*置不可分配位*/
    bits=memory_bitmap_length(CONFIG_MEMORY_POOL_PAGES)*sizeof(UINT64)-CONFIG_MEMORY_POOL_PAGES;
    index=memory_bitmap_length(CONFIG_MEMORY_POOL_PAGES)-1;
    while(bits>64)
    {
        cbitmap->bitmap[index]=MAX_UINT64;
        index--;
        bits=bits-64;
    }
    if(bits>0)
    {
        cbitmap->bitmap[index]|=~((1ULL<<(64-bits))-1);
    }
    *bitmap=cbitmap;

    /*构造TLSF元数据*/
    memory_tlsf_meta* cmeta=(memory_tlsf_meta*)(base+cbitmap->offset);
    cmeta->fl_bitmap=0;
    ZeroMem(cmeta->sl_bitmap,sizeof(cmeta->sl_bitmap));
    ZeroMem(cmeta->free,sizeof(cmeta->free));
    ZeroMem(cmeta->alloc,sizeof(cmeta->alloc));

    /*构造初始空闲区*/
    memory_tlsf_block* cblock=(memory_tlsf_block*)memory_init_free_offset(CONFIG_MEMORY_POOL_PAGES);
    cblock->psize=memory_tlsf_set_state(0,MEMORY_TLSF_BLOCK_ALLOC);
    cblock->csize=memory_tlsf_set_state(memory_init_free_size(CONFIG_MEMORY_POOL_PAGES,CONFIG_MEMORY_PREALLOCATED_PAGES)
        ,MEMORY_TLSF_BLOCK_FREE);
    cblock->magic=MEMORY_TLSF_MAGIC_FREE;

    /*添加该空闲块至对应空闲链表*/
    UINT8 fl=memory_fl_index(cblock->csize);
    UINT8 sl=memory_sl_index(cblock->csize,fl);
    cmeta->fl_bitmap=memory_tlsf_set_fl(cmeta->fl_bitmap,fl);
    cmeta->sl_bitmap[fl]=memory_tlsf_set_sl(cmeta->sl_bitmap[fl],sl);
    memory_list_add(cmeta->free[fl][sl], cblock);

    /*构造初始哨兵区*/
    memory_tlsf_block* csentinel=(memory_tlsf_block*)(memory_init_free_offset(CONFIG_MEMORY_POOL_PAGES)
        +memory_init_free_size(CONFIG_MEMORY_POOL_PAGES,CONFIG_MEMORY_PREALLOCATED_PAGES));
    csentinel->psize=cblock->csize;
    csentinel->csize=memory_tlsf_set_state(sizeof(memory_tlsf_block),MEMORY_TLSF_BLOCK_ALLOC);
    csentinel->magic=MEMORY_TLSF_MAGIC_ALLOC;
    memory_list_add(cmeta->alloc, cblock);
    *meta=cmeta;
    
    return EFI_SUCCESS;
}
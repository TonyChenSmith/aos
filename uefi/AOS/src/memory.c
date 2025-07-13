/* 
 * 模块“aos.uefi”内存池管理实现。
 * 实现了与内存池管理的相关函数，以及便于调试该部分功能的函数。
 * @date 2025-06-06
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "memory_internal.h"

/* 
 * 位图内存池。
 */
STATIC memory_bitmap* bitmap_pool=NULL;

/*
 * TLSF内存池。
 */
STATIC memory_tlsf_meta* tlsf_pool=NULL;

/* 
 * 获取大小对应第一级索引。
 * 
 * @param size 块大小。
 * 
 * @return 返回对应第一级索引。
 */
STATIC UINT8 EFIAPI inline memory_fl_index(IN UINTN size)
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
STATIC UINT8 EFIAPI inline memory_sl_index(IN UINTN size,IN UINT8 fl)
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
STATIC VOID EFIAPI memory_list_add(IN memory_tlsf_block** list,IN memory_tlsf_block* node)
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
STATIC VOID EFIAPI memory_list_remove(IN memory_tlsf_block** list,IN memory_tlsf_block* node)
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
 * 初始化内存池管理。
 * 
 * @param bitmap 位图地址。
 * @param meta   TLSF元数据地址。
 * 
 * @return 正常返回成功，异常返回对应错误。
 */
EFI_STATUS EFIAPI uefi_memory_init(OUT UINTN* bitmap,OUT UINTN* meta)
{
    EFI_STATUS status;

    /*分配内存池*/
    EFI_PHYSICAL_ADDRESS base;
    status=gBS->AllocatePages(AllocateAnyPages,EfiRuntimeServicesData,CONFIG_MEMORY_POOL_PAGES,&base);
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
    bits=memory_bitmap_length(CONFIG_MEMORY_POOL_PAGES)*64-CONFIG_MEMORY_POOL_PAGES;
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
    *bitmap=(UINTN)cbitmap;
    bitmap_pool=cbitmap;

    /*构造TLSF元数据*/
    memory_tlsf_meta* cmeta=(memory_tlsf_meta*)(base+cbitmap->offset);
    cmeta->fl_bitmap=0;
    ZeroMem(cmeta->sl_bitmap,sizeof(cmeta->sl_bitmap));
    ZeroMem(cmeta->free,sizeof(cmeta->free));
    ZeroMem(cmeta->alloc,sizeof(cmeta->alloc));
    cmeta->magic=MEMORY_TLSF_MAGIC_META;

    /*构造初始空闲区*/
    memory_tlsf_block* cblock=(memory_tlsf_block*)(base+memory_init_free_offset(CONFIG_MEMORY_POOL_PAGES));
    cblock->psize=memory_tlsf_set_state(0,MEMORY_TLSF_BLOCK_ALLOC);
    cblock->csize=memory_tlsf_set_state(memory_init_free_size(CONFIG_MEMORY_POOL_PAGES,
        CONFIG_MEMORY_PREALLOCATED_PAGES),MEMORY_TLSF_BLOCK_FREE);
    cblock->magic=MEMORY_TLSF_MAGIC_FREE;

    /*添加该空闲块至对应空闲链表*/
    UINT8 fl=memory_fl_index(cblock->csize);
    UINT8 sl=memory_sl_index(cblock->csize,fl);
    cmeta->fl_bitmap=memory_tlsf_set_fl(cmeta->fl_bitmap,fl);
    cmeta->sl_bitmap[fl]=memory_tlsf_set_sl(cmeta->sl_bitmap[fl],sl);
    memory_list_add(cmeta->free[fl][sl], cblock);

    /*构造初始哨兵区*/
    memory_tlsf_block* csentinel=(memory_tlsf_block*)(base+memory_init_free_offset(CONFIG_MEMORY_POOL_PAGES)+
        memory_init_free_size(CONFIG_MEMORY_POOL_PAGES,CONFIG_MEMORY_PREALLOCATED_PAGES));
    csentinel->psize=cblock->csize;
    csentinel->csize=memory_tlsf_set_state(sizeof(memory_tlsf_block),MEMORY_TLSF_BLOCK_ALLOC);
    csentinel->magic=MEMORY_TLSF_MAGIC_ALLOC;
    memory_list_add(cmeta->alloc, csentinel);
    *meta=(UINTN)cmeta;
    tlsf_pool=cmeta;
    
    return EFI_SUCCESS;
}

/* 
 * 在位图内分配连续多页。
 * 
 * @param pages 需要分配的连续页数。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI memory_page_alloc(IN UINTN pages)
{
    if(pages==0||bitmap_pool==NULL||bitmap_pool->free<pages)
    {
        return NULL;
    }

    UINTN start=0,count=0,page;
    BOOLEAN found=FALSE;
    for(page=CONFIG_MEMORY_PREALLOCATED_PAGES;page<bitmap_pool->size;page++)
    {
        if(!(bitmap_pool->bitmap[page>>6]&(1ULL<<(page&0x3F))))
        {
            if(count==0)
            {
                start=page;
            }
            count++;
            if(count>=pages)
            {
                found=TRUE;
                break;
            }
        }
        else
        {
            count=0;
        }
    }

    if(!found)
    {
        return NULL;
    }

    for(page=start;page<start+pages;page++)
    {
        bitmap_pool->bitmap[page>>6]|=1ULL<<(page&0x3F);
    }

    bitmap_pool->free-=pages;
    return (VOID*)((UINTN)bitmap_pool+EFI_PAGES_TO_SIZE(start));
}

/* 
 * 在位图内释放页面。
 * 
 * @param ptr 页面基址。
 * 
 * @return 无返回值。不会释放预分配区页面与错误地址。
 */
VOID EFIAPI memory_page_free(IN VOID* ptr)
{
    UINTN ptrn=(UINTN)ptr;
    if(ptr==NULL||ptrn&0xFFF)
    {
        return;
    }
    UINTN start=(UINTN)bitmap_pool+EFI_PAGES_TO_SIZE(CONFIG_MEMORY_PREALLOCATED_PAGES);
    UINTN end=(UINTN)bitmap_pool+EFI_PAGES_TO_SIZE(CONFIG_MEMORY_POOL_PAGES);
    if(start>ptrn||end<=ptrn)
    {
        return;
    }
    UINTN page=EFI_SIZE_TO_PAGES(ptrn-(UINTN)bitmap_pool);
    bitmap_pool->bitmap[page>>6]&=~(1ULL<<(page&0x3F));
    bitmap_pool->free++;
}

/* 
 * 将申请大小按8字节对齐，并对大小不足24的补足成24。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
STATIC UINTN EFIAPI memory_align_size(IN UINTN size)
{
    if(size<24)
    {
        return 24;
    }
    return ALIGN_VALUE(size,8);
}

/* 
 * 将TLSF块从链表中删除，并按情况进行分配。
 * 
 * @param size 申请所需大小。
 * @param node TLSF块结点。
 * @param fl   TLSF块结点链表所在第一级。
 * @param sl   TLSF块结点链表所在第二级。
 * 
 * @return 必然返回数据区地址。
 */
STATIC VOID* EFIAPI memory_block_alloc(IN UINTN size,IN memory_tlsf_block* node,IN UINT8 fl,IN UINT8 sl)
{
    memory_list_remove(tlsf_pool->free[fl][sl],node);

    if(tlsf_pool->free[fl][sl][0]==NULL)
    {
        tlsf_pool->sl_bitmap[fl]=memory_tlsf_clear_sl(tlsf_pool->sl_bitmap[fl],sl);
        if(tlsf_pool->sl_bitmap[fl]==0)
        {
            tlsf_pool->fl_bitmap=memory_tlsf_clear_fl(tlsf_pool->fl_bitmap,fl);
        }
    }
    
    memory_tlsf_block* next=(memory_tlsf_block*)((UINTN)node+memory_tlsf_get_size(node->csize));
    if(memory_tlsf_get_size(node->csize)-size>=64)
    {
        memory_tlsf_block* free=(memory_tlsf_block*)((UINTN)node+size);
        free->psize=memory_tlsf_set_state(size,MEMORY_TLSF_BLOCK_ALLOC);
        free->csize=memory_tlsf_set_state(memory_tlsf_get_size(node->csize)-size,MEMORY_TLSF_BLOCK_FREE);
        free->magic=MEMORY_TLSF_MAGIC_FREE;
        next->psize=free->csize;
        UINT8 free_fl=memory_fl_index(free->csize);
        UINT8 free_sl=memory_sl_index(free->csize,free_fl);

        tlsf_pool->fl_bitmap=memory_tlsf_set_fl(tlsf_pool->fl_bitmap,free_fl);
        tlsf_pool->sl_bitmap[free_fl]=memory_tlsf_set_sl(tlsf_pool->sl_bitmap[free_fl],free_sl);
        memory_list_add(tlsf_pool->free[free_fl][free_sl],free);

        node->csize=free->psize;
    }
    else
    {
        node->csize=memory_tlsf_set_state(node->csize,MEMORY_TLSF_BLOCK_ALLOC);
        next->psize=node->csize;
    }

    node->magic=MEMORY_TLSF_MAGIC_ALLOC;
    memory_list_add(tlsf_pool->alloc,node);
    return (VOID*)((UINTN)node+sizeof(memory_tlsf_block));
}

/* 
 * 申请新页面尝试进行TLSF分配。
 * 
 * @param size 申请所需大小。
 * 
 * @return 申请成功返回数据区地址，申请失败返回空。
 */
STATIC VOID* EFIAPI memory_new_block_alloc(IN UINTN size)
{
    UINTN pages=EFI_SIZE_TO_PAGES(size+sizeof(memory_tlsf_block));

    memory_tlsf_block* free=(memory_tlsf_block*)memory_page_alloc(pages);
    if(free==NULL)
    {
        return NULL;
    }
    free->magic=MEMORY_TLSF_MAGIC_FREE;
    free->psize=memory_tlsf_set_state(0,MEMORY_TLSF_BLOCK_ALLOC);
    free->csize=memory_tlsf_set_state(EFI_PAGES_TO_SIZE(pages)-sizeof(memory_tlsf_block),MEMORY_TLSF_BLOCK_FREE);

    UINT8 fl=memory_fl_index(free->csize);
    UINT8 sl=memory_sl_index(free->csize,fl);
    tlsf_pool->fl_bitmap=memory_tlsf_set_fl(tlsf_pool->fl_bitmap,fl);
    tlsf_pool->sl_bitmap[fl]=memory_tlsf_set_sl(tlsf_pool->sl_bitmap[fl],sl);
    memory_list_add(tlsf_pool->free[fl][sl],free);

    memory_tlsf_block* sentinel=(memory_tlsf_block*)((UINTN)free+memory_tlsf_get_size(free->csize));
    sentinel->psize=free->csize;
    sentinel->csize=memory_tlsf_set_state(sizeof(memory_tlsf_block),MEMORY_TLSF_BLOCK_ALLOC);
    sentinel->magic=MEMORY_TLSF_MAGIC_ALLOC;
    memory_list_add(tlsf_pool->alloc,sentinel);
    
    return memory_block_alloc(size,free,fl,sl);
}

/* 
 * 在TLSF内存池内申请内存。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI memory_pool_alloc(IN UINTN size)
{
    if(tlsf_pool==NULL||size==0)
    {
        return NULL;
    }

    size=memory_align_size(size);
    UINTN need=size+sizeof(memory_tlsf_block);
    UINT8 fl=memory_fl_index(need);
    UINT8 sl=memory_sl_index(need,fl);

    /*尝试寻找并取出空闲块。*/
    if(memory_tlsf_get_fl(tlsf_pool->fl_bitmap,fl))
    {
        if(memory_tlsf_get_sl(tlsf_pool->sl_bitmap[fl],sl))
        {
            memory_tlsf_block* node=tlsf_pool->free[fl][sl][1];
            while(node!=NULL&&memory_tlsf_get_size(node->csize)<need)
            {
                node=node->prev;
            }

            if(node!=NULL)
            {
                return memory_block_alloc(need,node,fl,sl);
            }
            else
            {
                UINT8 sl_bitmap=tlsf_pool->sl_bitmap[fl]&(MAX_UINT8&(MAX_UINT8<<(sl+1)));
                if(sl_bitmap!=0)
                {
                    sl=LowBitSet32(sl_bitmap);
                    return memory_block_alloc(need,tlsf_pool->free[fl][sl][1],fl,sl);
                }
                else
                {
                    UINT32 fl_bitmap=tlsf_pool->fl_bitmap&(MAX_UINT32<<(fl+1));
                    if(fl_bitmap!=0)
                    {
                        fl=LowBitSet32(fl_bitmap);
                        sl=LowBitSet32(tlsf_pool->sl_bitmap[fl]);
                        return memory_block_alloc(need,tlsf_pool->free[fl][sl][1],fl,sl);
                    }
                    else
                    {
                        return memory_new_block_alloc(need);
                    }
                }
            }
        }
        else
        {
            UINT8 sl_bitmap=tlsf_pool->sl_bitmap[fl]&(MAX_UINT8&(MAX_UINT8<<(sl+1)));
            if(sl_bitmap!=0)
            {
                sl=LowBitSet32(sl_bitmap);
                return memory_block_alloc(need,tlsf_pool->free[fl][sl][1],fl,sl);
            }
            else
            {
                UINT32 fl_bitmap=tlsf_pool->fl_bitmap&(MAX_UINT32<<(fl+1));
                if(fl_bitmap!=0)
                {
                    fl=LowBitSet32(fl_bitmap);
                    sl=LowBitSet32(tlsf_pool->sl_bitmap[fl]);
                    return memory_block_alloc(need,tlsf_pool->free[fl][sl][1],fl,sl);
                }
                else
                {
                    return memory_new_block_alloc(need);
                }
            }
        }
    }
    else
    {
        UINT32 fl_bitmap=tlsf_pool->fl_bitmap&(MAX_UINT32<<(fl+1));
        if(fl_bitmap!=0)
        {
            fl=LowBitSet32(fl_bitmap);
            sl=LowBitSet32(tlsf_pool->sl_bitmap[fl]);
            return memory_block_alloc(need,tlsf_pool->free[fl][sl][1],fl,sl);
        }
        else
        {
            return memory_new_block_alloc(need);
        }
    }
}

/* 
 * 向TLSF内存池内释放内存。
 * 
 * @param ptr 块数据基址。
 * 
 * @return 无返回值。对错误指针无反应。
 */
VOID EFIAPI memory_pool_free(IN VOID* ptr)
{
    if(ptr==NULL)
    {
        return;
    }
    memory_tlsf_block* node=(memory_tlsf_block*)((UINTN)ptr-sizeof(memory_tlsf_block));
    if(node->magic!=MEMORY_TLSF_MAGIC_ALLOC)
    {
        return;
    }

    /*尝试合并后续*/
    memory_list_remove(tlsf_pool->alloc,node);
    memory_tlsf_block* phys_next=(memory_tlsf_block*)((UINTN)node+memory_tlsf_get_size(node->csize));
    node->magic=MEMORY_TLSF_MAGIC_FREE;
    if(memory_tlsf_get_state(phys_next->csize))
    {
        ASSERT(phys_next->magic==MEMORY_TLSF_MAGIC_FREE);

        UINT8 next_fl=memory_fl_index(phys_next->csize);
        UINT8 next_sl=memory_sl_index(phys_next->csize,next_fl);

        memory_list_remove(tlsf_pool->free[next_fl][next_sl],phys_next);
        if(tlsf_pool->free[next_fl][next_sl][0]==NULL)
        {
            tlsf_pool->sl_bitmap[next_fl]=memory_tlsf_clear_sl(tlsf_pool->sl_bitmap[next_fl],next_sl);
            if(tlsf_pool->sl_bitmap[next_fl]==0)
            {
                tlsf_pool->fl_bitmap=memory_tlsf_clear_fl(tlsf_pool->fl_bitmap,next_fl);
            }
        }

        ASSERT((memory_tlsf_get_size(node->csize)+memory_tlsf_get_size(phys_next->csize))%8==0);

        node->csize=memory_tlsf_set_state(memory_tlsf_get_size(node->csize)+memory_tlsf_get_size(phys_next->csize),
            MEMORY_TLSF_BLOCK_FREE);
        phys_next=(memory_tlsf_block*)((UINTN)phys_next+memory_tlsf_get_size(phys_next->csize));
        phys_next->psize=node->csize;
    }
    else
    {
        ASSERT(phys_next->magic==MEMORY_TLSF_MAGIC_ALLOC);

        node->csize=memory_tlsf_set_state(node->csize,MEMORY_TLSF_BLOCK_FREE);
        phys_next->psize=node->csize;
    }

    /*尝试合并前继*/
    if(memory_tlsf_get_state(node->psize))
    {
        memory_tlsf_block* phys_prev=(memory_tlsf_block*)((UINTN)node-memory_tlsf_get_size(node->psize));
        
        ASSERT(phys_prev->magic==MEMORY_TLSF_MAGIC_FREE);

        UINT8 prev_fl=memory_fl_index(phys_prev->csize);
        UINT8 prev_sl=memory_sl_index(phys_prev->csize,prev_fl);

        memory_list_remove(tlsf_pool->free[prev_fl][prev_sl],phys_prev);
        if(tlsf_pool->free[prev_fl][prev_sl][0]==NULL)
        {
            tlsf_pool->sl_bitmap[prev_fl]=memory_tlsf_clear_sl(tlsf_pool->sl_bitmap[prev_fl],prev_sl);
            if(tlsf_pool->sl_bitmap[prev_fl]==0)
            {
                tlsf_pool->fl_bitmap=memory_tlsf_clear_fl(tlsf_pool->fl_bitmap,prev_fl);
            }
        }

        ASSERT((memory_tlsf_get_size(node->csize)+memory_tlsf_get_size(phys_prev->csize))%8==0);
        
        phys_prev->csize=memory_tlsf_set_state(memory_tlsf_get_size(node->csize)+memory_tlsf_get_size(phys_prev->csize),
            MEMORY_TLSF_BLOCK_FREE);

        node=phys_prev;
        phys_next->psize=node->csize;
    }

    /*对后续申请页进行释放*/
    if(phys_next->csize==sizeof(memory_tlsf_block)&&node->psize==0)
    {
        ASSERT(phys_next->magic==MEMORY_TLSF_MAGIC_ALLOC);

        if((UINTN)node>=(EFI_PAGES_TO_SIZE(CONFIG_MEMORY_PREALLOCATED_PAGES)+(UINTN)bitmap_pool))
        {
            memory_list_remove(tlsf_pool->alloc,phys_next);
            VOID* ptr=node;
            UINTN count=EFI_SIZE_TO_PAGES(memory_tlsf_get_size(node->csize)+sizeof(memory_tlsf_block));
            for(UINTN index=0;index<count;index++)
            {
                memory_page_free(ptr);
                ptr=(VOID*)((UINTN)ptr+SIZE_4KB);
            }
            return;
        }
    }
    
    /*默认行为*/
    UINT8 node_fl=memory_fl_index(node->csize);
    UINT8 node_sl=memory_sl_index(node->csize,node_fl);
    tlsf_pool->fl_bitmap=memory_tlsf_set_fl(tlsf_pool->fl_bitmap,node_fl);
    tlsf_pool->sl_bitmap[node_fl]=memory_tlsf_set_sl(tlsf_pool->sl_bitmap[node_fl],node_sl);
    memory_list_add(tlsf_pool->free[node_fl][node_sl],node);
}

/* 
 * 在调试模式转储内存池信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI memory_dump_pool_info(VOID)
{
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Memory Pool Dump\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Pool Base: 0x%016X.\n",(UINTN)bitmap_pool));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Page Bitmap Pool\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] TLSF Meta Offset: 0x%08X.\n",bitmap_pool->offset));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Page count: %u.\n",(UINT32)bitmap_pool->size));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Free pages: %u.\n",(UINT32)bitmap_pool->free));
    for(UINTN index=0;index<memory_bitmap_length(bitmap_pool->size);index++)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.memory] Bitmap-%lu: %016lX.\n",index,
            bitmap_pool->bitmap[index]));
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] TLSF Memory Pool\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] TLSF FL Bitmap: %08X.\n",tlsf_pool->fl_bitmap));
    for(UINTN index=0;index<32;index++)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.memory] TLSF SL Bitmap-%lu: %02X.\n",index,
            (UINT32)tlsf_pool->sl_bitmap[index]));
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] TLSF Allocated Memory List\n"));
    UINTN index=0;
    for(memory_tlsf_block* node=tlsf_pool->alloc[0];node!=NULL;node=node->next)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.memory] ----------\n"));
        DEBUG((DEBUG_INFO,"[aos.uefi.memory] Index-%lu: 0x%016lX.\n",index,(UINTN)node));
        DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Previous Size: 0x%016lX.\n",
            memory_tlsf_get_size(node->psize)));
        DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Current Size:  0x%016lX.\n",
            memory_tlsf_get_size(node->csize)));
        DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Current Magic: 0x%016lX.\n",node->magic));
        DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Previous Node: 0x%016lX.\n",(UINTN)node->prev));
        DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Next Node:     0x%016lX.\n",(UINTN)node->next));
        index++;
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] TLSF Free Memory List State\n"));
    for(UINTN fl=0;fl<32;fl++)
    {
        for(UINTN sl=0;sl<8;sl++)
        {
            if(tlsf_pool->free[fl][sl][0]==NULL)
            {
            }
            else
            {
                DEBUG((DEBUG_INFO,"[aos.uefi.memory] --------------------\n"));
                DEBUG((DEBUG_INFO,"[aos.uefi.memory] List-(%lu,%lu): 0x%016lX.\n",fl,sl,
                    (UINTN)tlsf_pool->free[fl][sl][0]));
                UINTN index=0;
                for(memory_tlsf_block* node=tlsf_pool->free[fl][sl][0];node!=NULL;node=node->next)
                {
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory] ----------\n"));
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Index-%lu: 0x%016lX.\n",index,
                        (UINTN)node));
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Previous Size: 0x%016X.\n",
                        memory_tlsf_get_size(node->psize)));
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Current Size:  0x%016X.\n",
                        memory_tlsf_get_size(node->csize)));
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Current Magic: 0x%016lX.\n",
                        node->magic));
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Previous Node: 0x%016lX.\n",
                        (UINTN)node->prev));
                    DEBUG((DEBUG_INFO,"[aos.uefi.memory]    Next Node:     0x%016lX.\n",
                        (UINTN)node->next));
                    index++;
                }
            }
        }
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] ==================================================\n"));
    DEBUG_CODE_END();
}

/* 
 * 内存池功能测试。
 * 
 * @return 无返回值。
 */
VOID EFIAPI memory_function_test(VOID)
{
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Init state.\n"));
    memory_dump_pool_info();
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Allocate 3 pools.\n"));
    VOID* a=memory_pool_alloc(23);
    VOID* b=memory_pool_alloc(180);
    VOID* c=memory_pool_alloc(8196);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] A pool: 0x%016X.\n",(UINTN)a));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] B pool: 0x%016X.\n",(UINTN)b));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] C pool: 0x%016X.\n",(UINTN)c));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] A state.\n"));
    memory_dump_pool_info();
    memory_pool_free(a);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] B state.\n"));
    memory_dump_pool_info();
    memory_pool_free(c);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] C state.\n"));
    memory_dump_pool_info();
    memory_pool_free(b);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] D state.\n"));
    memory_dump_pool_info();
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] Allocate 3 pools.\n"));
    a=memory_pool_alloc(SIZE_256KB+1042);
    b=memory_pool_alloc(180);
    c=memory_pool_alloc(1011);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] A pool: 0x%016X.\n",(UINTN)a));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] B pool: 0x%016X.\n",(UINTN)b));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] C pool: 0x%016X.\n",(UINTN)c));
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] A state.\n"));
    memory_dump_pool_info();
    memory_pool_free(b);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] B state.\n"));
    memory_dump_pool_info();
    memory_pool_free(a);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] C state.\n"));
    memory_dump_pool_info();
    memory_pool_free(c);
    DEBUG((DEBUG_INFO,"[aos.uefi.memory] D state.\n"));
    memory_dump_pool_info();
    DEBUG_CODE_END();
}

/* 
 * 检查数组中每个地址对应的内存类型。不可达区域标记为保留。调用者有义务保证两个数组的空间存在。
 * 
 * @param addrs  内存地址数组。
 * @param length 数组长度。
 * @param types  内存类型数组。
 * 
 * @return 返回调用状态。
 */
EFI_STATUS EFIAPI memory_get_memory_type(IN UINTN* addrs,IN UINTN length,OUT EFI_MEMORY_TYPE* types)
{
    EFI_MEMORY_DESCRIPTOR* memmap=NULL;
    UINTN map_size=0,desc_size,map_key;
    UINT32 desc_ver;
    EFI_STATUS status;

    if(length==0)
    {
        return EFI_SUCCESS;
    }
    else
    {
        for(UINTN index=0;index<length;index++)
        {
            types[index]=MEMORY_TYPE_OS_RESERVED_MAX;
        }
    }

    status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&desc_size,&desc_ver);
    if(status!=EFI_BUFFER_TOO_SMALL)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.memory] Incorrect parameters were set "
            "when checking the memory map size.\n"));
        return status;
    }
    else
    {
        ASSERT(map_size>0);
        memmap=memory_pool_alloc(map_size);
        if(memmap==NULL)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.memory] The configured memory pool space "
                "is insufficient.\n"));
            return EFI_OUT_OF_RESOURCES;
        }
        status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&desc_size,&desc_ver);
        if(status!=EFI_SUCCESS||desc_ver!=EFI_MEMORY_DESCRIPTOR_VERSION)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.memory] An unexpected condition occurred "
                "while obtaining the memory map.\n"));
            return EFI_UNSUPPORTED;
        }

        EFI_MEMORY_DESCRIPTOR* desc=memmap;
        UINTN end=map_size+(UINTN)memmap;
        while((UINTN)desc<end)
        {
            UINTN desc_end=EFI_PAGES_TO_SIZE(desc->NumberOfPages)+desc->PhysicalStart;
            for(UINTN index=0;index<length;index++)
            {
                if(types[index]==MEMORY_TYPE_OS_RESERVED_MAX)
                {
                    if(desc->PhysicalStart<=addrs[index]&&addrs[index]<desc_end)
                    {
                        types[index]=desc->Type;
                    }
                }
            }
            desc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)desc+desc_size);
        }

        for(UINTN index=0;index<length;index++)
        {
            if(types[index]==MEMORY_TYPE_OS_RESERVED_MAX)
            {
                types[index]=EfiReservedMemoryType;
            }
        }
        memory_pool_free(memmap);
        return EFI_SUCCESS;
    }
}
/* 
 * 模块“aos.uefi”内存池管理功能。
 * 实现了相关函数，以及便于调试的函数。
 * @date 2025-06-06
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "memi.h"

/*
 * 启动内存池。
 */
STATIC mem_tlsf_meta* boot_pool=NULL;

/* 
 * 获取大小对应第一级索引。
 * 
 * @param size 块大小。
 * 
 * @return 返回对应第一级索引。
 */
STATIC UINT8 EFIAPI inline mem_fl_index(IN UINTN size)
{
    size=mem_tlsf_get_size(size);
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
STATIC UINT8 EFIAPI inline mem_sl_index(IN UINTN size,IN UINT8 fl)
{
    size=mem_tlsf_get_size(size);
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
STATIC VOID EFIAPI mem_list_add(IN mem_tlsf_block** list,IN mem_tlsf_block* node)
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
STATIC VOID EFIAPI mem_list_remove(IN mem_tlsf_block** list,IN mem_tlsf_block* node)
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
 * 使用内存空间构造一个内存池。调用者有义务保证参数的合理性。
 * 
 * @param addr 内存块地址。
 * @param size 内存块大小。
 * 
 * @return 正常返回成功，大小不足最低要求返回错误。
 */
EFI_STATUS EFIAPI create_pool(IN VOID* addr,IN UINTN size)
{
    if(size<(sizeof(mem_tlsf_meta)+2*sizeof(mem_tlsf_block)+24))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.mem] Due to insufficient memory block size, "
            "the construction of the memory pool failed."));
        return EFI_OUT_OF_RESOURCES;
    }

    mem_tlsf_meta* meta=(mem_tlsf_meta*)addr;
    ZeroMem(meta,sizeof(mem_tlsf_meta));
    meta->magic=MEM_TLSF_MAGIC_META;

    mem_tlsf_block* block=(mem_tlsf_block*)((UINTN)addr+sizeof(mem_tlsf_meta));
    block->psize=mem_tlsf_set_state(0,MEM_TLSF_BLOCK_ALLOC);
    block->csize=mem_tlsf_set_state(size-sizeof(mem_tlsf_meta)-sizeof(mem_tlsf_block),MEM_TLSF_BLOCK_FREE);
    block->magic=MEM_TLSF_MAGIC_FREE;

    UINT8 fl=mem_fl_index(block->csize);
    UINT8 sl=mem_sl_index(block->csize,fl);
    meta->fl_bitmap=mem_tlsf_set_fl(meta->fl_bitmap,fl);
    meta->sl_bitmap[fl]=mem_tlsf_set_sl(meta->sl_bitmap[fl],sl);
    mem_list_add(meta->free[fl][sl],block);

    mem_tlsf_block* sentinel=(mem_tlsf_block*)((UINTN)addr+size-sizeof(mem_tlsf_block));
    sentinel->psize=block->csize;
    sentinel->csize=mem_tlsf_set_state(sizeof(mem_tlsf_block),MEM_TLSF_BLOCK_ALLOC);
    sentinel->magic=MEM_TLSF_MAGIC_ALLOC;
    mem_list_add(meta->alloc,sentinel);

    return EFI_SUCCESS;
}

/* 
 * 初始化内存池管理功能。这里管理的是引导内存池。
 * 
 * @param bpool 引导内存池基址。
 * 
 * @return 正常返回成功，异常返回对应错误。
 */
EFI_STATUS EFIAPI mem_init(OUT UINTN* bpool)
{
    EFI_STATUS status;

    EFI_PHYSICAL_ADDRESS base=SIZE_2GB;
    status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,CONFIG_BOOTSTRAP_POOL,&base);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.mem] Failed to allocate bootstrap memory pool "
            "of %llu pages.\n",(UINT64)CONFIG_BOOTSTRAP_POOL));
        return status;
    }

    status=create_pool((VOID*)base,EFI_PAGES_TO_SIZE(CONFIG_BOOTSTRAP_POOL));
    if(EFI_ERROR(status))
    {
        return status;
    }
    *bpool=base;

    boot_pool=(mem_tlsf_meta*)base;
    return EFI_SUCCESS;
}

/* 
 * 将空闲内存块从内存池空闲链表中删除，并按情况进行分配。
 * 
 * @param pool 内存池基址。
 * @param size 申请所需大小。
 * @param node TLSF块结点。
 * @param fl   TLSF块结点链表所在第一级。
 * @param sl   TLSF块结点链表所在第二级。
 * 
 * @return 必然返回数据区地址。
 */
STATIC VOID* EFIAPI mem_block_alloc(IN mem_tlsf_meta* pool,IN UINTN size,IN mem_tlsf_block* node,
    IN UINT8 fl,IN UINT8 sl)
{
    mem_list_remove(pool->free[fl][sl],node);

    if(pool->free[fl][sl][0]==NULL)
    {
        pool->sl_bitmap[fl]=mem_tlsf_clear_sl(pool->sl_bitmap[fl],sl);
        if(pool->sl_bitmap[fl]==0)
        {
            pool->fl_bitmap=mem_tlsf_clear_fl(pool->fl_bitmap,fl);
        }
    }
    
    mem_tlsf_block* next=(mem_tlsf_block*)((UINTN)node+mem_tlsf_get_size(node->csize));
    if(mem_tlsf_get_size(node->csize)-size>=64)
    {
        mem_tlsf_block* free=(mem_tlsf_block*)((UINTN)node+size);
        free->psize=mem_tlsf_set_state(size,MEM_TLSF_BLOCK_ALLOC);
        free->csize=mem_tlsf_set_state(mem_tlsf_get_size(node->csize)-size,MEM_TLSF_BLOCK_FREE);
        free->magic=MEM_TLSF_MAGIC_FREE;
        next->psize=free->csize;
        UINT8 free_fl=mem_fl_index(free->csize);
        UINT8 free_sl=mem_sl_index(free->csize,free_fl);

        pool->fl_bitmap=mem_tlsf_set_fl(pool->fl_bitmap,free_fl);
        pool->sl_bitmap[free_fl]=mem_tlsf_set_sl(pool->sl_bitmap[free_fl],free_sl);
        mem_list_add(pool->free[free_fl][free_sl],free);

        node->csize=free->psize;
    }
    else
    {
        node->csize=mem_tlsf_set_state(node->csize,MEM_TLSF_BLOCK_ALLOC);
        next->psize=node->csize;
    }

    node->magic=MEM_TLSF_MAGIC_ALLOC;
    mem_list_add(pool->alloc,node);
    return (VOID*)((UINTN)node+sizeof(mem_tlsf_block));
}

/* 
 * 在内存池内申请一块内存。
 * 
 * @param pool 内存池基址。
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI pool_alloc(IN VOID* pool,IN UINTN size)
{
    mem_tlsf_meta* tpool=(mem_tlsf_meta*)pool;
    if(pool==NULL||size==0||tpool->magic!=MEM_TLSF_MAGIC_META)
    {
        return NULL;
    }

    size=mem_align_size(size);
    UINTN need=size+sizeof(mem_tlsf_block);
    UINT8 fl=mem_fl_index(need);
    UINT8 sl=mem_sl_index(need,fl);

    if(mem_tlsf_get_fl(tpool->fl_bitmap,fl))
    {
        if(mem_tlsf_get_sl(tpool->sl_bitmap[fl],sl))
        {
            mem_tlsf_block* node=tpool->free[fl][sl][1];
            while(node!=NULL&&mem_tlsf_get_size(node->csize)<need)
            {
                node=node->prev;
            }

            if(node!=NULL)
            {
                return mem_block_alloc(tpool,need,node,fl,sl);
            }
            else
            {
                UINT8 sl_bitmap=tpool->sl_bitmap[fl]&(MAX_UINT8&(MAX_UINT8<<(sl+1)));
                if(sl_bitmap!=0)
                {
                    sl=LowBitSet32(sl_bitmap);
                    return mem_block_alloc(tpool,need,tpool->free[fl][sl][1],fl,sl);
                }
                else
                {
                    UINT32 fl_bitmap=tpool->fl_bitmap&(MAX_UINT32<<(fl+1));
                    if(fl_bitmap!=0)
                    {
                        fl=LowBitSet32(fl_bitmap);
                        sl=LowBitSet32(tpool->sl_bitmap[fl]);
                        return mem_block_alloc(tpool,need,tpool->free[fl][sl][1],fl,sl);
                    }
                    else
                    {
                        /*没有自主扩展能力*/
                        return NULL;
                    }
                }
            }
        }
        else
        {
            UINT8 sl_bitmap=tpool->sl_bitmap[fl]&(MAX_UINT8&(MAX_UINT8<<(sl+1)));
            if(sl_bitmap!=0)
            {
                sl=LowBitSet32(sl_bitmap);
                return mem_block_alloc(tpool,need,tpool->free[fl][sl][1],fl,sl);
            }
            else
            {
                UINT32 fl_bitmap=tpool->fl_bitmap&(MAX_UINT32<<(fl+1));
                if(fl_bitmap!=0)
                {
                    fl=LowBitSet32(fl_bitmap);
                    sl=LowBitSet32(tpool->sl_bitmap[fl]);
                    return mem_block_alloc(pool,need,tpool->free[fl][sl][1],fl,sl);
                }
                else
                {
                    /*没有自主扩展能力*/
                    return NULL;
                }
            }
        }
    }
    else
    {
        UINT32 fl_bitmap=tpool->fl_bitmap&(MAX_UINT32<<(fl+1));
        if(fl_bitmap!=0)
        {
            fl=LowBitSet32(fl_bitmap);
            sl=LowBitSet32(tpool->sl_bitmap[fl]);
            return mem_block_alloc(tpool,need,tpool->free[fl][sl][1],fl,sl);
        }
        else
        {
            /*没有自主扩展能力*/
            return NULL;
        }
    }
}

/* 
 * 在引导内存池内申请一块内存。
 * 
 * @param size 申请大小。
 * 
 * @return 分配成功返回对应地址，分配失败返回空。
 */
VOID* EFIAPI malloc(IN UINTN size)
{
    return pool_alloc(boot_pool,size);
}

/* 
 * 向内存池内释放内存。调用者有义务保证释放指针归属于该内存池，为了性能不做指针大范围检查。
 * 
 * @param pool 内存池基址。
 * @param ptr  内存块基址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI pool_free(IN VOID* pool,IN VOID* ptr)
{
    mem_tlsf_meta* tpool=(mem_tlsf_meta*)pool;
    if(pool==NULL||ptr==NULL||tpool->magic!=MEM_TLSF_MAGIC_META)
    {
        return;
    }
    mem_tlsf_block* node=(mem_tlsf_block*)((UINTN)ptr-sizeof(mem_tlsf_block));
    if(node->magic!=MEM_TLSF_MAGIC_ALLOC)
    {
        return;
    }

    /*合并后续*/
    mem_list_remove(tpool->alloc,node);
    mem_tlsf_block* phys_next=(mem_tlsf_block*)((UINTN)node+mem_tlsf_get_size(node->csize));
    node->magic=MEM_TLSF_MAGIC_FREE;
    if(mem_tlsf_get_state(phys_next->csize))
    {
        ASSERT(phys_next->magic==MEM_TLSF_MAGIC_FREE);

        UINT8 next_fl=mem_fl_index(phys_next->csize);
        UINT8 next_sl=mem_sl_index(phys_next->csize,next_fl);

        mem_list_remove(tpool->free[next_fl][next_sl],phys_next);
        if(tpool->free[next_fl][next_sl][0]==NULL)
        {
            tpool->sl_bitmap[next_fl]=mem_tlsf_clear_sl(tpool->sl_bitmap[next_fl],next_sl);
            if(tpool->sl_bitmap[next_fl]==0)
            {
                tpool->fl_bitmap=mem_tlsf_clear_fl(tpool->fl_bitmap,next_fl);
            }
        }

        ASSERT((mem_tlsf_get_size(node->csize)+mem_tlsf_get_size(phys_next->csize))%8==0);

        node->csize=mem_tlsf_set_state(mem_tlsf_get_size(node->csize)+mem_tlsf_get_size(phys_next->csize),
            MEM_TLSF_BLOCK_FREE);
        phys_next=(mem_tlsf_block*)((UINTN)phys_next+mem_tlsf_get_size(phys_next->csize));
        phys_next->psize=node->csize;
    }
    else
    {
        ASSERT(phys_next->magic==MEM_TLSF_MAGIC_ALLOC);

        node->csize=mem_tlsf_set_state(node->csize,MEM_TLSF_BLOCK_FREE);
        phys_next->psize=node->csize;
    }

    /*合并前继*/
    if(mem_tlsf_get_state(node->psize))
    {
        mem_tlsf_block* phys_prev=(mem_tlsf_block*)((UINTN)node-mem_tlsf_get_size(node->psize));
        
        ASSERT(phys_prev->magic==MEM_TLSF_MAGIC_FREE);

        UINT8 prev_fl=mem_fl_index(phys_prev->csize);
        UINT8 prev_sl=mem_sl_index(phys_prev->csize,prev_fl);

        mem_list_remove(tpool->free[prev_fl][prev_sl],phys_prev);
        if(tpool->free[prev_fl][prev_sl][0]==NULL)
        {
            tpool->sl_bitmap[prev_fl]=mem_tlsf_clear_sl(tpool->sl_bitmap[prev_fl],prev_sl);
            if(tpool->sl_bitmap[prev_fl]==0)
            {
                tpool->fl_bitmap=mem_tlsf_clear_fl(tpool->fl_bitmap,prev_fl);
            }
        }

        ASSERT((mem_tlsf_get_size(node->csize)+mem_tlsf_get_size(phys_prev->csize))%8==0);
        
        phys_prev->csize=mem_tlsf_set_state(mem_tlsf_get_size(node->csize)+mem_tlsf_get_size(phys_prev->csize),
            MEM_TLSF_BLOCK_FREE);

        node=phys_prev;
        phys_next->psize=node->csize;
    }
    
    /*默认行为*/
    UINT8 node_fl=mem_fl_index(node->csize);
    UINT8 node_sl=mem_sl_index(node->csize,node_fl);
    tpool->fl_bitmap=mem_tlsf_set_fl(tpool->fl_bitmap,node_fl);
    tpool->sl_bitmap[node_fl]=mem_tlsf_set_sl(tpool->sl_bitmap[node_fl],node_sl);
    mem_list_add(tpool->free[node_fl][node_sl],node);
}

/* 
 * 向引导内存池内释放内存。调用者有义务保证释放指针归属于该内存池，为了性能不做指针大范围检查。
 * 
 * @param ptr 内存块基址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI free(IN VOID* ptr)
{
    return pool_free(boot_pool,ptr);
}

/* 
 * 在调试模式转储输入的内存池信息。
 * 
 * @param pool 内存池基址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_pool_info(IN VOID* pool)
{
    DEBUG_CODE_BEGIN();
    mem_tlsf_meta* meta=(mem_tlsf_meta*)pool;
    if(meta==NULL||meta->magic!=MEM_TLSF_MAGIC_META)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.mem] The address is not a memory pool.\n"));
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] Memory Pool Dump\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] Pool Base: 0x%016X.\n",(UINTN)pool));
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] TLSF FL Bitmap: %08X.\n",meta->fl_bitmap));
    for(UINTN index=0;index<32;index++)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.mem] TLSF SL Bitmap-%lu: %02X.\n",index,
            (UINT32)meta->sl_bitmap[index]));
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] TLSF Allocated Memory List\n"));
    UINTN index=0;
    for(mem_tlsf_block* node=meta->alloc[0];node!=NULL;node=node->next)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.mem] ----------\n"));
        DEBUG((DEBUG_INFO,"[aos.uefi.mem] Index-%lu: 0x%016lX.\n",index,(UINTN)node));
        DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Previous Size: 0x%016lX.\n",
            mem_tlsf_get_size(node->psize)));
        DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Current Size:  0x%016lX.\n",
            mem_tlsf_get_size(node->csize)));
        DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Current Magic: 0x%016lX.\n",node->magic));
        DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Previous Node: 0x%016lX.\n",(UINTN)node->prev));
        DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Next Node:     0x%016lX.\n",(UINTN)node->next));
        index++;
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] TLSF Free Memory List State\n"));
    for(UINTN fl=0;fl<32;fl++)
    {
        for(UINTN sl=0;sl<8;sl++)
        {
            if(meta->free[fl][sl][0]==NULL)
            {
            }
            else
            {
                DEBUG((DEBUG_INFO,"[aos.uefi.mem] --------------------\n"));
                DEBUG((DEBUG_INFO,"[aos.uefi.mem] List-(%lu,%lu): 0x%016lX.\n",fl,sl,
                    (UINTN)meta->free[fl][sl][0]));
                UINTN index=0;
                for(mem_tlsf_block* node=meta->free[fl][sl][0];node!=NULL;node=node->next)
                {
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem] ----------\n"));
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem] Index-%lu: 0x%016lX.\n",index,
                        (UINTN)node));
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Previous Size: 0x%016X.\n",
                        mem_tlsf_get_size(node->psize)));
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Current Size:  0x%016X.\n",
                        mem_tlsf_get_size(node->csize)));
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Current Magic: 0x%016lX.\n",
                        node->magic));
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Previous Node: 0x%016lX.\n",
                        (UINTN)node->prev));
                    DEBUG((DEBUG_INFO,"[aos.uefi.mem]    Next Node:     0x%016lX.\n",
                        (UINTN)node->next));
                    index++;
                }
            }
        }
    }
    DEBUG((DEBUG_INFO,"[aos.uefi.mem] ==================================================\n"));
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
EFI_STATUS EFIAPI get_memory_type(IN UINTN* addrs,IN UINTN length,OUT EFI_MEMORY_TYPE* types)
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
        DEBUG((DEBUG_ERROR,"[aos.uefi.mem] Incorrect parameters were set "
            "when checking the memory map size.\n"));
        return status;
    }
    else
    {
        ASSERT(map_size>0);
        memmap=malloc(map_size);
        if(memmap==NULL)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.mem] The configured memory pool space "
                "is insufficient.\n"));
            return EFI_OUT_OF_RESOURCES;
        }
        status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&desc_size,&desc_ver);
        if(status!=EFI_SUCCESS||desc_ver!=EFI_MEMORY_DESCRIPTOR_VERSION)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.mem] An unexpected condition occurred "
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
        free(memmap);
        return EFI_SUCCESS;
    }
}
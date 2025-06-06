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

    EFI_PHYSICAL_ADDRESS base;
    status=gBS->AllocatePages(AllocateAnyPages,EfiLoaderData,CONFIG_MEMORY_POOL_PAGES,&base);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.memory] Failed to allocate memory pool of %llu pages.\n",
            (UINT64)CONFIG_MEMORY_POOL_PAGES));
        return status;
    }

    memory_bitmap* cbitmap=(memory_bitmap*)base;
    cbitmap->size=CONFIG_MEMORY_POOL_PAGES;
    cbitmap->free=CONFIG_MEMORY_POOL_PAGES;
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

    memory_tlsf_meta* cmeta=(memory_tlsf_meta*)(base+cbitmap->offset);
    
    return EFI_SUCCESS;
}
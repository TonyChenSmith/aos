/**
 * 模块页表与线性区管理。
 * @date 2025-07-13
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "pvmi.h"

/**
 * 位图指针。
 */
STATIC UINT8* bitmap;

/**
 * 位图长度。
 */
STATIC UINTN bitmap_length;

/**
 * 页表池基址。
 */
STATIC UINTN pbase;

/**
 * 页表池页数。
 */
STATIC UINTN ppages;

/**
 * 申请一页内存。
 * 
 * @return 正常返回一页指针基址，错误返回固定常量。
 */
STATIC VOID* EFIAPI pvm_page_alloc()
{
    UINTN index=bitmap_length;
    for(UINTN i=0;i<bitmap_length;i++)
    {
        if(bitmap[i]==0xFF)
        {
            continue;
        }
        else
        {
            index=i;
            break;
        }
    }

    if(index>=bitmap_length)
    {
        /*页面内存池耗尽，无页面可用。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] "
            "The page memory pool is exhausted and no pages are available.\n"));
        return PVM_ERROR;
    }
    else
    {
        UINT8 bit=8;
        for(UINTN i=0;i<8;i++)
        {
            if(bitmap[index]&PVM_BITMAP_MASK[i])
            {
                continue;
            }
            else
            {
                bit=i;
                break;
            }
        }
        bitmap[index]|=PVM_BITMAP_MASK[bit];
        index=(index<<3)+bit;
        VOID* addr=(VOID*)((index<<12)+pbase);
        ZeroMem(addr,SIZE_4KB);
        return addr;
    }
}

/**
 * 释放一页内存。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_page_free(IN VOID* addr)
{
    UINTN limit=pbase+(ppages<<12);
    UINTN ptr=(UINTN)addr;
    if(ptr<pbase||ptr>=limit||(ptr&0xFFF))
    {
        return;
    }

    ptr=(ptr-pbase)>>12;
    UINT8 bit=ptr&0x7;
    ptr=ptr>>3;
    bitmap[ptr]=bitmap[ptr]&(~PVM_BITMAP_MASK[bit]);
}

/**
 * 遍历位图信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_bitmap()
{
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Bitmap Pool Base:0x%016lX\n",pbase));
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Bitmap Pool Size:%lu\n",ppages));
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Bitmap Info\n"));
    for(UINTN index=0;index<bitmap_length;index++)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.pvm] %02lu:%02X\n",index,bitmap[index]));
    }
    DEBUG_CODE_END();
}

/**
 * 不可执行位。
 */
STATIC BOOLEAN nx=FALSE;

/**
 * 大页位。
 */
STATIC BOOLEAN page1gb=FALSE;

/**
 * 将物理内存块映射到页表项内。
 * 
 * @param pt    页表地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 一定成功。
 */
STATIC EFI_STATUS EFIAPI pvm_pte_map(IN VOID* pt,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    UINTN index=(vaddr>>12)&0x1FF;
    while(pages>0)
    {
        UINT64 pte=flags&PVM_PTE_4K_FLAGS_MASK;
        pte|=paddr|PVM_PTE_P;
        ((UINT64*)pt)[index]=pte;
        index++;
        paddr+=SIZE_4KB;
        pages--;
    }
    return EFI_SUCCESS;
}

/**
 * 将物理内存块映射到页目录项内。
 * 
 * @param pd    页目录地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 正常映射返回成功，页面申请失败返回资源耗尽。
 */
STATIC UINTN EFIAPI pvm_pde_map(IN VOID* pd,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>21)&0x1FF;
        UINTN block=0x200-((vaddr>>12)&0x1FF);
        block=MIN(block,pages);
        UINT64 pde;
        if(block==0x200&&!(vaddr&PVM_PAGE_2M_OFFSET_MASK||paddr&PVM_PAGE_2M_OFFSET_MASK))
        {
            pde=flags&PVM_PTE_2M_FLAGS_MASK;
            pde|=PVM_PTE_P|PVM_PDE_PS|paddr;
            ((UINT64*)pd)[index]=pde;
        }
        else
        {
            pde=((UINT64*)pd)[index];
            VOID* pt;
            if(pde&PVM_PTE_P)
            {
                pt=(VOID*)(pde&PVM_PTE_ADDR_MASK);
            }
            else
            {
                pt=pvm_page_alloc();
                if((UINTN)pt&0xFFF)
                {
                    return EFI_OUT_OF_RESOURCES;
                }
                pde=PVM_PDE_FLAGS;
                pde|=(UINT64)pt;
                ((UINT64*)pd)[index]=pde;
            }
            pvm_pte_map(pt,vaddr,paddr,block,flags);
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
    return EFI_SUCCESS;
}

/**
 * 将物理内存块映射到页目录指针项内。
 * 
 * @param pdp   页目录指针地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 正常映射返回成功，页面申请失败返回资源耗尽。
 */
STATIC EFI_STATUS EFIAPI pvm_pdpe_map(IN VOID* pdp,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>30)&0x1FF;
        UINTN block=0x40000-((vaddr>>12)&0x3FFFF);
        block=MIN(block,pages);
        UINT64 pdpe;
        if(page1gb&&block==0x40000&&!(vaddr&PVM_PAGE_1G_OFFSET_MASK||paddr&PVM_PAGE_1G_OFFSET_MASK))
        {
            pdpe=flags&PVM_PTE_1G_FLAGS_MASK;
            pdpe|=PVM_PTE_P|PVM_PDE_PS|paddr;
            ((UINT64*)pdp)[index]=pdpe;
        }
        else
        {
            pdpe=((UINT64*)pdp)[index];
            VOID* pd;
            if(pdpe&PVM_PTE_P)
            {
                pd=(VOID*)(pdpe&PVM_PTE_ADDR_MASK);
            }
            else
            {
                pd=pvm_page_alloc();
                if((UINTN)pd&0xFFF)
                {
                    return EFI_OUT_OF_RESOURCES;
                }
                pdpe=PVM_PDE_FLAGS;
                pdpe|=(UINT64)pd;
                ((UINT64*)pdp)[index]=pdpe;
            }
            if(EFI_ERROR(pvm_pde_map(pd,vaddr,paddr,block,flags)))
            {
                return EFI_OUT_OF_RESOURCES;
            }
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
    return EFI_SUCCESS;
}

/**
 * 将物理内存块映射到4级页映射项内。
 * 
 * @param pml4  4级页映射地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 正常映射返回成功，页面申请失败返回资源耗尽。
 */
STATIC EFI_STATUS EFIAPI pvm_pml4e_map(IN VOID* pml4,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>39)&0x1FF;
        UINTN block=0x8000000-((vaddr>>12)&0x7FFFFFF);
        block=MIN(block,pages);
        UINT64 pml4e=((UINT64*)pml4)[index];
        VOID* pdp;
        if(pml4e&PVM_PTE_P)
        {
            pdp=(VOID*)(pml4e&PVM_PTE_ADDR_MASK);
        }
        else
        {
            pdp=pvm_page_alloc();
            if((UINTN)pdp&0xFFF)
            {
                return EFI_OUT_OF_RESOURCES;
            }
            pml4e=PVM_PDE_FLAGS;
            pml4e|=(UINT64)pdp;
            ((UINT64*)pml4)[index]=pml4e;
        }
        if(EFI_ERROR(pvm_pdpe_map(pdp,vaddr,paddr,block,flags)))
        {
            return EFI_OUT_OF_RESOURCES;
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
    return EFI_SUCCESS;
}

/**
 * 将物理内存块映射到5级页映射内。
 * 
 * @param pml5  5级页映射地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 正常映射返回成功，页面申请失败返回资源耗尽。
 */
STATIC EFI_STATUS EFIAPI pvm_pml5e_map(IN VOID* pml5,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>48)&0x1FF;
        UINTN block=0x1000000000ULL-((vaddr>>12)&0xFFFFFFFFFULL);
        block=MIN(block,pages);
        UINT64 pml5e=((UINT64*)pml5)[index];
        VOID* pml4;
        if(pml5e&PVM_PTE_P)
        {
            pml4=(VOID*)(pml5e&PVM_PTE_ADDR_MASK);
        }
        else
        {
            pml4=pvm_page_alloc();
            if((UINTN)pml4&0xFFF)
            {
                return EFI_OUT_OF_RESOURCES;
            }
            pml5e=PVM_PDE_FLAGS;
            pml5e|=(UINT64)pml4;
            ((UINT64*)pml5)[index]=pml5e;
        }
        if(EFI_ERROR(pvm_pml4e_map(pml4,vaddr,paddr,block,flags)))
        {
            return EFI_OUT_OF_RESOURCES;
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
    return EFI_SUCCESS;
}

/**
 * 返回页表是否为空。
 * 
 * @param pt    页表地址。
 * 
 * @return 为空返回真。
 */
STATIC BOOLEAN EFIAPI pvm_pt_is_empty(IN VOID* pt)
{
    UINT64* pte=(UINT64*)pt;
    UINTN index=0;
    while(index<512)
    {
        if(pte[index]&PVM_PTE_P)
        {
            return FALSE;
        }
        index++;
    }
    return TRUE;
}

/**
 * 将页表内指定线性地址解除映射。
 * 
 * @param pt    页表地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_pte_unmap(IN VOID* pt,IN UINTN vaddr,IN UINTN pages)
{
    UINTN index=(vaddr>>12)&0x1FF;
    while(pages>0)
    {
        ((UINT64*)pt)[index]=0;
        index++;
        pages--;
    }
}

/**
 * 将页目录内指定线性地址解除映射。
 * 
 * @param pd    页目录地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_pde_unmap(IN VOID* pd,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>21)&0x1FF;
        UINTN block=0x200-((vaddr>>12)&0x1FF);
        block=MIN(block,pages);
        UINT64 pde=((UINT64*)pd)[index];
        if(pde&PVM_PTE_P)
        {
            if(pde&PVM_PDE_PS)
            {
                ((UINT64*)pd)[index]=0;
            }
            else
            {
                VOID* pt=(VOID*)(pde&PVM_PTE_ADDR_MASK);
                pvm_pte_unmap(pt,vaddr,block);
                if(pvm_pt_is_empty(pt))
                {
                    pvm_page_free(pt);
                    ((UINT64*)pd)[index]=0;
                }
            }
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
    }
}

/**
 * 将页目录指针内指定线性地址解除映射。
 * 
 * @param pdp   页目录指针地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_pdpe_unmap(IN VOID* pdp,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>30)&0x1FF;
        UINTN block=0x40000-((vaddr>>12)&0x3FFFF);
        block=MIN(block,pages);
        UINT64 pdpe=((UINT64*)pdp)[index];
        if(pdpe&PVM_PTE_P)
        {
            if(pdpe&PVM_PDE_PS)
            {
                ((UINT64*)pdp)[index]=0;
            }
            else
            {
                VOID* pd=(VOID*)(pdpe&PVM_PTE_ADDR_MASK);
                pvm_pde_unmap(pd,vaddr,block);
                if(pvm_pt_is_empty(pd))
                {
                    pvm_page_free(pd);
                    ((UINT64*)pdp)[index]=0;
                }
            }
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
    }
}

/**
 * 将4级页映射内指定线性地址解除映射。
 * 
 * @param pml4  4级页映射地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_pml4e_unmap(IN VOID* pml4,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>39)&0x1FF;
        UINTN block=0x8000000-((vaddr>>12)&0x7FFFFFF);
        block=MIN(block,pages);
        UINT64 pml4e=((UINT64*)pml4)[index];
        if(pml4e&PVM_PTE_P)
        {
            VOID* pdp=(VOID*)(pml4e&PVM_PTE_ADDR_MASK);
            pvm_pdpe_unmap(pdp,vaddr,block);
            if(pvm_pt_is_empty(pdp))
            {
                pvm_page_free(pdp);
                ((UINT64*)pml4)[index]=0;
            }
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
    }
}

/**
 * 将5级页映射内指定线性地址解除映射。
 * 
 * @param pml5  5级页映射地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_pml5e_unmap(IN VOID* pml5,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>48)&0x1FF;
        UINTN block=0x1000000000ULL-((vaddr>>12)&0xFFFFFFFFFULL);
        block=MIN(block,pages);
        UINT64 pml5e=((UINT64*)pml5)[index];
        if(pml5e&PVM_PTE_P)
        {
            VOID* pml4=(VOID*)(pml5e&PVM_PTE_ADDR_MASK);
            pvm_pml4e_unmap(pml4,vaddr,block);
            if(pvm_pt_is_empty(pml4))
            {
                pvm_page_free(pml4);
                ((UINT64*)pml5)[index]=0;
            }
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
    }
}

/**
 * 将线性区标志转换成映射标志。
 * 
 * @param flags 线性区标志。
 * 
 * @return 对应的映射标志。
 */
STATIC UINT64 EFIAPI pvm_flags_to_pflags(IN UINT64 flags)
{
    UINT64 pflags=0;

    if(flags&AOS_BOOT_VMA_USER)
    {
        pflags|=PVM_PTE_US;
    }

    if(flags&AOS_BOOT_VMA_GLOBAL)
    {
        pflags|=PVM_PTE_G;
    }

    UINT64 rwx=flags&AOS_BOOT_VMA_RWX_MASK;
    if(nx)
    {
        switch(rwx)
        {
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE:
                pflags|=PVM_PTE_RW;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE:
                pflags|=PVM_PTE_RW|PVM_PTE_NX;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_EXECUTE:
                break;
            case AOS_BOOT_VMA_READ:
            default:
                pflags|=PVM_PTE_NX;
                break;
        }
    }
    else
    {
        switch(rwx)
        {
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE:
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE:
                pflags|=PVM_PTE_RW;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_EXECUTE:
            case AOS_BOOT_VMA_READ:
            default:
                break;
        }
    }

    switch(flags&AOS_BOOT_VMA_TYPE_MASK)
    {
        case AOS_BOOT_VMA_TYPE_WB:
            break;
        case AOS_BOOT_VMA_TYPE_WP:
            pflags|=PVM_PDE_PAT|PVM_PTE_PAT|PVM_PTE_PWT;
            break;
        case AOS_BOOT_VMA_TYPE_WT:
            pflags|=PVM_PTE_PWT;
            break;
        case AOS_BOOT_VMA_TYPE_WC:
            pflags|=PVM_PDE_PAT|PVM_PTE_PAT;
            break;
        case AOS_BOOT_VMA_TYPE_UCM:
            pflags|=PVM_PTE_PCD;
            break;
        case AOS_BOOT_VMA_TYPE_UC:
        default:
            pflags|=PVM_PTE_PWT|PVM_PTE_PCD;
            break;
    }

    return pflags;
}

/**
 * 顶级页表根地址。
 */
STATIC VOID* page_table=NULL;

/**
 * 将物理地址范围映射到5级分页的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 映射状态。
 */
STATIC EFI_STATUS EFIAPI pvm_pml5_map(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(pages>EFI_SIZE_TO_PAGES(SIZE_128PB))
    {
        /*不支持的页数。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] The page count is not supported.\n"));
        return EFI_UNSUPPORTED;
    }

    UINTN mask=vaddr&PVM_VADDR5_RESERVED_MASK;
    if(mask!=0&&mask!=PVM_VADDR5_RESERVED_MASK)
    {
        /*地址不是规范地址。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address is not in canonical form.\n"));
        return EFI_UNSUPPORTED;
    }

    if(flags&AOS_BOOT_VMA_HUGEPAGE)
    {
        if((paddr&PVM_PAGE_2M_OFFSET_MASK)||(vaddr&PVM_PAGE_2M_OFFSET_MASK))
        {
            /*地址偏移区未对齐。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address offset region is not aligned.\n"));
            return EFI_UNSUPPORTED;
        }

        if(pages%EFI_SIZE_TO_PAGES(SIZE_2MB))
        {
            /*映射页数非大页倍数。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] "
                "Number of pages to map is not a multiple of a huge page.\n"));
            return EFI_UNSUPPORTED;
        }
    }

    UINTN plimit=paddr+EFI_PAGES_TO_SIZE(pages)-1;
    UINTN vlimit=vaddr+EFI_PAGES_TO_SIZE(pages)-1;

    if(plimit<paddr||vlimit<vaddr)
    {
        /*地址发生回绕。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address wrap-around occurred.\n"));
        return EFI_UNSUPPORTED;
    }

    UINT64 pflags=pvm_flags_to_pflags(flags);
    if(page_table!=NULL)
    {
        pvm_pml5e_unmap(page_table,vaddr,pages);
        if(EFI_ERROR(pvm_pml5e_map(page_table,vaddr,paddr,pages,pflags)))
        {
            /*可用发生回绕。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Out of available pages.\n"));
            pvm_pml5e_unmap(page_table,vaddr,pages);
            return EFI_OUT_OF_RESOURCES;
        }
    }

    return EFI_SUCCESS;
}

/**
 * 将物理地址范围映射到4级分页的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 映射状态。
 */
STATIC EFI_STATUS EFIAPI pvm_pml4_map(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(pages>EFI_SIZE_TO_PAGES(SIZE_256TB))
    {
        /*不支持的页数。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] The page count is not supported.\n"));
        return EFI_UNSUPPORTED;
    }

    UINTN mask=vaddr&PVM_VADDR4_RESERVED_MASK;
    if(mask!=0&&mask!=PVM_VADDR4_RESERVED_MASK)
    {
        /*地址不是规范地址。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address is not in canonical form.\n"));
        return EFI_UNSUPPORTED;
    }

    if(flags&AOS_BOOT_VMA_HUGEPAGE)
    {
        if((paddr&PVM_PAGE_2M_OFFSET_MASK)||(vaddr&PVM_PAGE_2M_OFFSET_MASK))
        {
            /*地址偏移区未对齐。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address offset region is not aligned.\n"));
            return EFI_UNSUPPORTED;
        }

        if(pages%EFI_SIZE_TO_PAGES(SIZE_2MB))
        {
            /*映射页数非大页倍数。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] "
                "Number of pages to map is not a multiple of a huge page.\n"));
            return EFI_UNSUPPORTED;
        }
    }

    UINTN plimit=paddr+EFI_PAGES_TO_SIZE(pages)-1;
    UINTN vlimit=vaddr+EFI_PAGES_TO_SIZE(pages)-1;

    if(plimit<paddr||vlimit<vaddr)
    {
        /*地址发生回绕。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address wrap-around occurred.\n"));
        return EFI_UNSUPPORTED;
    }

    UINT64 pflags=pvm_flags_to_pflags(flags);
    if(page_table!=NULL)
    {
        pvm_pml4e_unmap(page_table,vaddr,pages);
        if(EFI_ERROR(pvm_pml4e_map(page_table,vaddr,paddr,pages,pflags)))
        {
            /*可用发生回绕。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Out of available pages.\n"));
            pvm_pml4e_unmap(page_table,vaddr,pages);
            return EFI_OUT_OF_RESOURCES;
        }
    }

    return EFI_SUCCESS;
}

/**
 * 将物理地址范围映射到页表的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 映射状态。
 */
STATIC EFI_STATUS EFIAPI (*pvm_pml_map)(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/**
 * 解除5级分页内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 映射状态。
 */
STATIC EFI_STATUS EFIAPI pvm_pml5_unmap(IN UINTN start,IN UINTN end)
{
    UINTN pages=EFI_SIZE_TO_PAGES(end-start);

    pvm_pml5e_unmap(page_table,start,pages);

    return EFI_SUCCESS;
}

/**
 * 解除4级分页内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 映射状态。
 */
STATIC EFI_STATUS EFIAPI pvm_pml4_unmap(IN UINTN start,IN UINTN end)
{
    UINTN pages=EFI_SIZE_TO_PAGES(end-start);

    pvm_pml4e_unmap(page_table,start,pages);

    return EFI_SUCCESS;
}

/**
 * 解除页表内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 映射状态。
 */
STATIC EFI_STATUS EFIAPI (*pvm_pml_unmap)(IN UINTN start,IN UINTN end);

/**
 * 遍历页表内容。
 * 
 * @param pt    页表地址。
 * @param top   是否为顶级页表。
 * @param level 当前页表层级。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_dump_page_table(IN VOID* pt,IN BOOLEAN top,IN UINTN level)
{
    DEBUG_CODE_BEGIN();
    UINT64* pte=(UINT64*)pt;
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] ==================================================\n"));
    if(top)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Top Page Table: 0x%016lX\n",(UINTN)pt));
    }
    else
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Page Table: 0x%016lX\n",(UINTN)pt));
    }

    UINTN index;
    for(index=0;index<512;index++)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.pvm] %03lu: %016lX\n",index,pte[index]));
    }

    if(level==0)
    {
        return;
    }

    for(index=0;index<512;index++)
    {
        if((pte[index]&PVM_PTE_P)&&!(pte[index]&PVM_PDE_PS))
        {
            UINT64 lpt=pte[index]&PVM_PTE_ADDR_MASK;
            pvm_dump_page_table((VOID*)lpt,FALSE,level-1);
        }
    }
    DEBUG_CODE_END();
}

/**
 * 遍历5级页表。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_dump_pml5()
{
    pvm_dump_page_table(page_table,TRUE,4);
}

/**
 * 遍历4级页表。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_dump_pml4()
{
    pvm_dump_page_table(page_table,TRUE,3);
}

/**
 * 遍历顶级页表。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*pvm_dump_pml)();

/**
 * 遍历全部内核页表。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_page_table()
{
    pvm_dump_pml();
}

/**
 * 线性区链表头。
 */
STATIC aos_boot_vma** head=NULL;

/**
 * 线性区链表尾。
 */
STATIC aos_boot_vma** tail=NULL;

/**
 * 添加线性区到启动参数。
 * 
 * @param vma 线性区。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_add_vma_node(IN aos_boot_vma* vma)
{
    if(*tail==NULL)
    {
        *head=vma;
        *tail=vma;
    }
    else
    {
        aos_boot_vma* pnode=NULL;
        aos_boot_vma* cnode=*head;
        while(cnode!=NULL&&cnode->end<=vma->start)
        {
            pnode=cnode;
            cnode=cnode->next;
        }

        if(pnode==NULL)
        {
            cnode->prev=vma;
            vma->next=cnode;
            *head=vma;
        }
        else if(cnode==NULL)
        {
            pnode->next=vma;
            vma->prev=pnode;
            *tail=vma;
        }
        else
        {
            vma->prev=pnode;
            vma->next=cnode;
            pnode->next=vma;
            cnode->prev=vma;
        }
    }
}

/**
 * 在启动参数删除一个线性区。删除依据以指针一致为准。
 * 
 * @param vma 线性区。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_remove_vma_node(IN aos_boot_vma* vma)
{
    if(*head!=NULL)
    {
        aos_boot_vma* node=*head;
        while(node!=vma)
        {
            node=node->next;
        }

        if(node->prev==NULL)
        {
            *head=node->next;
        }
        else
        {
            node->prev->next=node->next;
        }
        if(node->next==NULL)
        {
            *tail=node->prev;
        }
        else
        {
            node->next->prev=node->prev;
        }

        node->prev=NULL;
        node->next=NULL;
    }
}

/**
 * 通过线性地址寻找对应的线性区结点。
 * 
 * @param vaddr 线性区域地址。
 * 
 * @return 找到返回结点地址，未找到返回空。
 */
STATIC aos_boot_vma* EFIAPI pvm_find_vma_node(IN UINTN vaddr)
{
    aos_boot_vma* node=*head;
    UINTN start,end;

    while(node!=NULL)
    {
        start=node->start;
        end=node->end-1;

        if(start>vaddr)
        {
            return NULL;
        }
        else if(vaddr<=end)
        {
            return node;
        }
        node=node->next;
    }

    return NULL;
}

/**
 * 检查输入的线性区域内是否存在于已有的线性区。
 * 
 * @param vaddr 线性区域地址。
 * @param pages 线性区域页数。
 * 
 * @return 有重叠区域返回真。
 */
STATIC BOOLEAN EFIAPI pvm_check_vma_node(IN UINTN vaddr,IN UINTN pages)
{
    aos_boot_vma* node=*head;
    UINTN start,end;

    UINTN limit=vaddr+EFI_PAGES_TO_SIZE(pages)-1;

    while(node!=NULL)
    {
        start=node->start;
        end=node->end-1;

        if(start>=limit)
        {
            return FALSE;
        }
        else if(vaddr<end)
        {
            return TRUE;
        }
        else
        {
            node=node->next;
        }
    }

    return FALSE;
}

/**
 * 添加一个内核线性区。
 * 
 * @param vaddr 线性区域基址。
 * @param paddr 物理区域基址。
 * @param pages 区域页数。
 * @param flags 线性区标记。
 * 
 * @return 正常返回成功。出现问题返回对应错误。
 */
EFI_STATUS EFIAPI add_kernel_vma(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(pages==0)
    {
        /*不支持的页数。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] The page count is not supported.\n"));
        return EFI_UNSUPPORTED;
    }

    if((paddr&PVM_PAGE_4K_OFFSET_MASK)||(vaddr&PVM_PAGE_4K_OFFSET_MASK))
    {
        /*地址偏移区未对齐。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address offset region is not aligned.\n"));
        return EFI_UNSUPPORTED;
    }

    if(paddr&PVM_PADDR_RESERVED_MASK)
    {
        /*地址超出页表管理范围。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Address is out of page table management range.\n"));
        return EFI_UNSUPPORTED;
    }

    if(pvm_check_vma_node(vaddr,pages))
    {
        /*添加的线性区与已有线性区重叠。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] The added VMA overlaps with existing VMAs.\n"));
        return EFI_UNSUPPORTED;
    }

    aos_boot_vma* vma=umalloc(sizeof(aos_boot_vma));
    if(vma==NULL)
    {
        /*内存池内没有足够的空间申请线性区。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] "
            "Insufficient space in memory pool for VMA allocation.\n"));
        return EFI_OUT_OF_RESOURCES;
    }

    if(EFI_ERROR(pvm_pml_map(vaddr,paddr,pages,flags)))
    {
        ufree(vma);
        return EFI_UNSUPPORTED;
    }
    else
    {
        vma->start=vaddr;
        vma->end=vaddr+EFI_PAGES_TO_SIZE(pages);
        vma->flags=flags|AOS_BOOT_VMA_ALLOCATED;
        vma->prev=NULL;
        vma->next=NULL;
        pvm_add_vma_node(vma);
        return EFI_SUCCESS;
    }
}

/**
 * 删除一个内核线性区。仅在通过输入参数能够找到线性区时才会删除。
 * 
 * @param vaddr 线性区域地址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI remove_kernel_vma(IN UINTN vaddr)
{
    aos_boot_vma* node=pvm_find_vma_node(vaddr);
    if(node!=NULL)
    {
        pvm_pml_unmap(node->start,node->end);
        pvm_remove_vma_node(node);
        ufree(node);
    }
}

/**
 * 检查一个线性区域是否与存在的已有线性区重叠。
 * 
 * @param vaddr 线性区域地址。
 * @param pages 线性区域页数。从实用角度，调用者有必要输入非零值。
 * 
 * @return 有重叠区域返回真。
 */
BOOLEAN EFIAPI check_vma_overlap(IN UINTN vaddr,IN UINTN pages)
{
    if(pages==0)
    {
        return FALSE;
    }
    return pvm_check_vma_node(vaddr,pages);
}

/**
 * 遍历所有线性区信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_vma()
{
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.pvm] VMA Info\n"));
    if(*head==NULL)
    {
        DEBUG((DEBUG_INFO,"[aos.uefi.pvm] ==================================================\n"));
        DEBUG((DEBUG_INFO,"[aos.uefi.pvm] No VMA.\n"));
    }
    else
    {
        aos_boot_vma* node=*head;
        UINTN index=0;
        while(node!=NULL)
        {
            DEBUG((DEBUG_INFO,"[aos.uefi.pvm] ==================================================\n"));
            DEBUG((DEBUG_INFO,"[aos.uefi.pvm] VMA-%lu\n",index));
            DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Range:0x%016lX-0x%016lX\n",node->start,node->end));
            DEBUG((DEBUG_INFO,"[aos.uefi.pvm] Flags:%016lX\n",node->flags));
            node=node->next;
            index++;
        }
    }
    DEBUG_CODE_END();
}

/**
 * 生成随机32位数。内部增强算法，但由于熵源单一还是容易被撞击。参数通过指针传递，方便后续二次加强计算。
 * 
 * @param a 参数A。
 * @param b 参数B。
 * @param c 参数C。
 * 
 * @return 返回一个随机数。
 */
UINT32 EFIAPI random32(IN OUT UINT32* a,IN OUT UINT32* b,IN OUT UINT32* c)
{
    RandomBytes((UINT8*)a,sizeof(UINT32));
    MicroSecondDelay((*a&0xF)+1);
    RandomBytes((UINT8*)b,sizeof(UINT32));
    MicroSecondDelay((*b&0xF)+2);
    RandomBytes((UINT8*)c,sizeof(UINT32));

    UINT32 value=LRotU32(*a,13);
    value^=*b;
    value=LRotU32(value,17);
    value^=*c;
    value=value*PVM_RANDOM_MAGIC;
    value^=value>>16;

    return value;
}

/**
 * 初始化页表与线性区管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 正常设置线性区返回成功。
 */
EFI_STATUS EFIAPI pvm_init(IN OUT aos_boot_params* params)
{
    pbase=params->ppool_base;
    bitmap=params->bitmap;
    bitmap_length=params->bitmap_length;
    ppages=params->ppool_pages;
    head=&params->vma_head;
    tail=&params->vma_tail;

    nx=params->features.features&AOS_FEATURES_NX;
    page1gb=params->features.features&AOS_FEATURES_PAGE1GB;
    if(params->state.state&AOS_STATE_LA57)
    {
        pvm_pml_map=pvm_pml5_map;
        pvm_pml_unmap=pvm_pml5_unmap;
        pvm_dump_pml=pvm_dump_pml5;
    }
    else
    {
        pvm_pml_map=pvm_pml4_map;
        pvm_pml_unmap=pvm_pml4_unmap;
        pvm_dump_pml=pvm_dump_pml4;
    }

    /*必不可能报错。*/
    page_table=pvm_page_alloc();
    params->page_table=(UINTN)page_table;

    /*以下为线性区添加*/
    EFI_STATUS status;
    status=add_kernel_vma(0,0,EFI_SIZE_TO_PAGES(SIZE_4GB),
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE|AOS_BOOT_VMA_TYPE_UC);
    if(EFI_ERROR(status))
    {
        /*建立1：1映射线性区失败。*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Failed to create 1:1 mapping vma.\n"));
        return EFI_OUT_OF_RESOURCES;
    }

    return EFI_SUCCESS;
}
/* 
 * 模块“aos.uefi”页表与线性区管理功能。
 * 实现了相关函数，以及便于调试的函数。
 * @date 2025-07-13
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "pvmi.h"

/* 
 * 添加线性区到启动参数。
 * 
 * @param params 启动参数。
 * @param vma    线性区。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_add_vma(IN OUT aos_boot_params* params,IN aos_boot_vma* vma)
{
    if(params->vma_tail==NULL)
    {
        params->vma_head=vma;
        params->vma_tail=vma;
    }
    else
    {
        params->vma_tail->next=vma;
        vma->prev=params->vma_tail;
        params->vma_tail=vma;
    }
}

/* 
 * 在启动参数删除一个线性区。删除依据以指针一致为准。
 * 
 * @param params 启动参数。
 * @param vma    线性区。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI pvm_remove_vma(IN OUT aos_boot_params* params,IN aos_boot_vma* vma)
{
    if(params->vma_head!=NULL)
    {
        aos_boot_vma* node=params->vma_head;
        while(node!=vma)
        {
            node=node->next;
        }

        if(node->prev==NULL)
        {
            params->vma_head=node->next;
        }
        else
        {
            node->prev->next=node->next;
        }
        if(node->next==NULL)
        {
            params->vma_tail=node->prev;
        }
        else
        {
            node->next->prev=node->prev;
        }

        node->prev=NULL;
        node->next=NULL;
    }
}

/* 
 * 位图指针。
 */
STATIC UINT8* bitmap;

/* 
 * 位图长度。
 */
STATIC UINTN bitmap_length;

/* 
 * 页表池基址。
 */
STATIC UINTN pbase;

/* 
 * 页表池页数。
 */
STATIC UINTN ppages;

/* 
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
        index=(index<<3)+bit;
        VOID* addr=(VOID*)((index<<12)+pbase);
        ZeroMem(addr,SIZE_4KB);
        return addr;
    }
}

/* 
 * 不可执行位。
 */
STATIC BOOLEAN nx=FALSE;

/* 
 * 大页位。
 */
STATIC BOOLEAN page1gb=FALSE;

/* 
 * 5级分页。
 */
STATIC BOOLEAN ptl5=FALSE;

/* 
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

/* 
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

/* 
 * 将物理内存块映射到页表内。其后续存在性维护不归映射函数管理。
 * 
 * @param pml   顶级页表地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 正常映射返回成功，页面申请失败返回资源耗尽。
 */
STATIC EFI_STATUS EFIAPI (*pvm_pml_map)(IN VOID* pml,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/* 
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

/* 
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
            pde|=PVM_PTE_P|PVM_PDE_PS;
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

/* 
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
            pdpe|=PVM_PTE_P|PVM_PDE_PS;
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

/* 
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

/* 
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

/* 
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

/* 
 * 将页表内指定线性地址解除映射。
 * 
 * @param pml   顶级页表地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*pvm_pml_unmap)(IN VOID* pml,IN UINTN vaddr,IN UINTN pages);

/* 
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

/* 
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

/* 
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

/* 
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

/* 
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

/* 
 * 初始化页表与线性区管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 应该返回成功。
 */
EFI_STATUS EFIAPI pvm_init(IN OUT aos_boot_params* params)
{
    pbase=params->ppool_base;
    bitmap=params->bitmap;
    bitmap_length=params->bitmap_length;
    ppages=params->ppool_pages;

    nx=params->features.features&AOS_FEATURES_NX;
    page1gb=params->features.features&AOS_FEATURES_PAGE1GB;
    if(params->state.state&AOS_STATE_LA57)
    {
        pvm_pml_map=pvm_pml5e_map;
        pvm_pml_unmap=pvm_pml5e_unmap;
        ptl5=TRUE;
    }
    else
    {
        pvm_pml_map=pvm_pml4e_map;
        pvm_pml_unmap=pvm_pml4e_unmap;
    }
    return EFI_SUCCESS;
}
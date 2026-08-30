/**
 * 模块分页管理。
 * @date 2026-08-07
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "config.h"
#include "mark.h"
#include "panic.h"
#include "params.h"
#include "pool.h"

/**
 * 位图用掩码。
 */
CONST UINT8 PAGE_BITMAP_MASK[]={BIT0,BIT1,BIT2,BIT3,BIT4,BIT5,BIT6,BIT7};

/**
 * 页表项使用位。
 */
#define PAGE_PTE_P BIT0
#define PAGE_PTE_RW BIT1
#define PAGE_PTE_US BIT2
#define PAGE_PTE_PWT BIT3
#define PAGE_PTE_PCD BIT4
#define PAGE_PTE_A BIT5
#define PAGE_PTE_D BIT6
#define PAGE_PTE_PAT BIT7
#define PAGE_PTE_G BIT8
#define PAGE_PTE_NX BIT63

/**
 * 页目录项使用位。
 */
#define PAGE_PDE_PS BIT7
#define PAGE_PDE_PAT BIT12

/**
 * 页面地址偏移掩码。
 */
#define PAGE_4K_OFFSET_MASK 0xFFF
#define PAGE_2M_OFFSET_MASK 0x1FFFFF
#define PAGE_1G_OFFSET_MASK 0x3FFFFFFF

/**
 * 页表项地址掩码。
 */
#define PAGE_PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

/**
 * 页表项使用标志掩码。
 */
#define PAGE_PTE_4K_FLAGS_MASK (PAGE_PTE_RW|PAGE_PTE_US|PAGE_PTE_PWT|PAGE_PTE_PCD|PAGE_PTE_PAT|PAGE_PTE_G|PAGE_PTE_NX)
#define PAGE_PTE_2M_FLAGS_MASK (PAGE_PTE_RW|PAGE_PTE_US|PAGE_PTE_PWT|PAGE_PTE_PCD|PAGE_PTE_G|PAGE_PDE_PAT|PAGE_PTE_NX)
#define PAGE_PTE_1G_FLAGS_MASK (PAGE_PTE_RW|PAGE_PTE_US|PAGE_PTE_PWT|PAGE_PTE_PCD|PAGE_PTE_G|PAGE_PDE_PAT|PAGE_PTE_NX)

/**
 * 页目录项使用标志。
 */
#define PAGE_PDE_FLAGS (PAGE_PTE_P|PAGE_PTE_RW|PAGE_PTE_US)

/**
 * 物理地址保留掩码。
 */
#define PAGE_PADDR_RESERVED_MASK 0xFFF0000000000000ULL

/**
 * 4级分页线性地址保留掩码。
 */
#define PAGE_VADDR4_RESERVED_MASK 0xFFFF800000000000ULL

/**
 * 5级分页线性地址保留掩码。
 */
#define PAGE_VADDR5_RESERVED_MASK 0xFF00000000000000ULL

/**
 * 页表位图。
 */
STATIC UINT8* ptmap;

/**
 * 页表位图长度。
 */
STATIC UINTN ptmap_size;

/**
 * 页表池基址。
 */
STATIC UINTN ptbase;

/**
 * 页表池页数。
 */
STATIC UINTN ptpages;

/**
 * 不可执行位。
 */
STATIC BOOLEAN nx=FALSE;

/**
 * 大页位。
 */
STATIC BOOLEAN page1gb=FALSE;

/**
 * 顶级页表根地址。
 */
STATIC VOID* pt_root=NULL;

/**
 * 工作准备状态。
 */
STATIC BOOLEAN ready=FALSE;

/**
 * 将物理地址范围映射到页表的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*page_pml_map_handle)(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/**
 * 解除页表内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*page_pml_unmap_handle)(IN UINTN start,IN UINTN end);

/**
 * 申请一页内存。
 * 
 * @return 正常返回一页指针基址，错误返回固定常量。
 */
STATIC VOID* EFIAPI page_page_alloc()
{
    UINTN index=ptmap_size;
    for(UINTN i=0;i<ptmap_size;i++)
    {
        if(ptmap[i]==0xFF)
        {
            continue;
        }
        else
        {
            index=i;
            break;
        }
    }

    if(index>=ptmap_size)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate page from page table pool.");
    }
    else
    {
        UINT8 bit=8;
        for(UINTN i=0;i<8;i++)
        {
            if(ptmap[index]&PAGE_BITMAP_MASK[i])
            {
                continue;
            }
            else
            {
                bit=i;
                break;
            }
        }
        ptmap[index]|=PAGE_BITMAP_MASK[bit];
        index=(index<<3)+bit;
        VOID* addr=(VOID*)((index<<12)+ptbase);
        ZeroMem(addr,SIZE_4KB);
        return addr;
    }
}

/**
 * 释放一页内存。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_page_free(IN VOID* addr)
{
    UINTN limit=ptbase+(ptpages<<12);
    UINTN ptr=(UINTN)addr;
    if(ptr<ptbase||ptr>=limit||(ptr&0xFFF))
    {
        return;
    }

    ptr=(ptr-ptbase)>>12;
    UINT8 bit=ptr&0x7;
    ptr=ptr>>3;
    ptmap[ptr]=ptmap[ptr]&(~PAGE_BITMAP_MASK[bit]);
}

/**
 * 将物理内存块映射到页表项内。
 * 
 * @param pt    页表地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pte_map(IN VOID* pt,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    UINTN index=(vaddr>>12)&0x1FF;
    while(pages>0)
    {
        UINT64 pte=flags&PAGE_PTE_4K_FLAGS_MASK;
        pte|=paddr|PAGE_PTE_P;
        ((UINT64*)pt)[index]=pte;
        index++;
        paddr+=SIZE_4KB;
        pages--;
    }
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
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pde_map(IN VOID* pd,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>21)&0x1FF;
        UINTN block=0x200-((vaddr>>12)&0x1FF);
        block=MIN(block,pages);
        UINT64 pde;
        if(block==0x200&&!(vaddr&PAGE_2M_OFFSET_MASK||paddr&PAGE_2M_OFFSET_MASK))
        {
            pde=flags&PAGE_PTE_2M_FLAGS_MASK;
            pde|=PAGE_PTE_P|PAGE_PDE_PS|paddr;
            ((UINT64*)pd)[index]=pde;
        }
        else
        {
            pde=((UINT64*)pd)[index];
            VOID* pt;
            if(pde&PAGE_PTE_P)
            {
                pt=(VOID*)(pde&PAGE_PTE_ADDR_MASK);
            }
            else
            {
                pt=page_page_alloc();
                pde=PAGE_PDE_FLAGS;
                pde|=(UINT64)pt;
                ((UINT64*)pd)[index]=pde;
            }
            page_pte_map(pt,vaddr,paddr,block,flags);
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
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
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pdpe_map(IN VOID* pdp,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>30)&0x1FF;
        UINTN block=0x40000-((vaddr>>12)&0x3FFFF);
        block=MIN(block,pages);
        UINT64 pdpe;
        if(page1gb&&block==0x40000&&!(vaddr&PAGE_1G_OFFSET_MASK||paddr&PAGE_1G_OFFSET_MASK))
        {
            pdpe=flags&PAGE_PTE_1G_FLAGS_MASK;
            pdpe|=PAGE_PTE_P|PAGE_PDE_PS|paddr;
            ((UINT64*)pdp)[index]=pdpe;
        }
        else
        {
            pdpe=((UINT64*)pdp)[index];
            VOID* pd;
            if(pdpe&PAGE_PTE_P)
            {
                pd=(VOID*)(pdpe&PAGE_PTE_ADDR_MASK);
            }
            else
            {
                pd=page_page_alloc();
                pdpe=PAGE_PDE_FLAGS;
                pdpe|=(UINT64)pd;
                ((UINT64*)pdp)[index]=pdpe;
            }
            page_pde_map(pd,vaddr,paddr,block,flags);
        }
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
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
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pml4e_map(IN VOID* pml4,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>39)&0x1FF;
        UINTN block=0x8000000-((vaddr>>12)&0x7FFFFFF);
        block=MIN(block,pages);
        UINT64 pml4e=((UINT64*)pml4)[index];
        VOID* pdp;
        if(pml4e&PAGE_PTE_P)
        {
            pdp=(VOID*)(pml4e&PAGE_PTE_ADDR_MASK);
        }
        else
        {
            pdp=page_page_alloc();
            pml4e=PAGE_PDE_FLAGS;
            pml4e|=(UINT64)pdp;
            ((UINT64*)pml4)[index]=pml4e;
        }
        page_pdpe_map(pdp,vaddr,paddr,block,flags);
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
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
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pml5e_map(IN VOID* pml5,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>48)&0x1FF;
        UINTN block=0x1000000000ULL-((vaddr>>12)&0xFFFFFFFFFULL);
        block=MIN(block,pages);
        UINT64 pml5e=((UINT64*)pml5)[index];
        VOID* pml4;
        if(pml5e&PAGE_PTE_P)
        {
            pml4=(VOID*)(pml5e&PAGE_PTE_ADDR_MASK);
        }
        else
        {
            pml4=page_page_alloc();
            pml5e=PAGE_PDE_FLAGS;
            pml5e|=(UINT64)pml4;
            ((UINT64*)pml5)[index]=pml5e;
        }
        page_pml4e_map(pml4,vaddr,paddr,block,flags);
        pages-=block;
        vaddr+=EFI_PAGES_TO_SIZE(block);
        paddr+=EFI_PAGES_TO_SIZE(block);
    }
}

/**
 * 返回页表是否为空。
 * 
 * @param pt 页表地址。
 * 
 * @return 为空返回真。
 */
STATIC BOOLEAN EFIAPI page_pt_is_empty(IN VOID* pt)
{
    UINT64* pte=(UINT64*)pt;
    UINTN index=0;
    while(index<512)
    {
        if(pte[index]&PAGE_PTE_P)
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
STATIC VOID EFIAPI page_pte_unmap(IN VOID* pt,IN UINTN vaddr,IN UINTN pages)
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
STATIC VOID EFIAPI page_pde_unmap(IN VOID* pd,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>21)&0x1FF;
        UINTN block=0x200-((vaddr>>12)&0x1FF);
        block=MIN(block,pages);
        UINT64 pde=((UINT64*)pd)[index];
        if(pde&PAGE_PTE_P)
        {
            if(pde&PAGE_PDE_PS)
            {
                ((UINT64*)pd)[index]=0;
            }
            else
            {
                VOID* pt=(VOID*)(pde&PAGE_PTE_ADDR_MASK);
                page_pte_unmap(pt,vaddr,block);
                if(page_pt_is_empty(pt))
                {
                    page_page_free(pt);
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
STATIC VOID EFIAPI page_pdpe_unmap(IN VOID* pdp,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>30)&0x1FF;
        UINTN block=0x40000-((vaddr>>12)&0x3FFFF);
        block=MIN(block,pages);
        UINT64 pdpe=((UINT64*)pdp)[index];
        if(pdpe&PAGE_PTE_P)
        {
            if(pdpe&PAGE_PDE_PS)
            {
                ((UINT64*)pdp)[index]=0;
            }
            else
            {
                VOID* pd=(VOID*)(pdpe&PAGE_PTE_ADDR_MASK);
                page_pde_unmap(pd,vaddr,block);
                if(page_pt_is_empty(pd))
                {
                    page_page_free(pd);
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
STATIC VOID EFIAPI page_pml4e_unmap(IN VOID* pml4,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>39)&0x1FF;
        UINTN block=0x8000000-((vaddr>>12)&0x7FFFFFF);
        block=MIN(block,pages);
        UINT64 pml4e=((UINT64*)pml4)[index];
        if(pml4e&PAGE_PTE_P)
        {
            VOID* pdp=(VOID*)(pml4e&PAGE_PTE_ADDR_MASK);
            page_pdpe_unmap(pdp,vaddr,block);
            if(page_pt_is_empty(pdp))
            {
                page_page_free(pdp);
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
STATIC VOID EFIAPI page_pml5e_unmap(IN VOID* pml5,IN UINTN vaddr,IN UINTN pages)
{
    while(pages>0)
    {
        UINTN index=(vaddr>>48)&0x1FF;
        UINTN block=0x1000000000ULL-((vaddr>>12)&0xFFFFFFFFFULL);
        block=MIN(block,pages);
        UINT64 pml5e=((UINT64*)pml5)[index];
        if(pml5e&PAGE_PTE_P)
        {
            VOID* pml4=(VOID*)(pml5e&PAGE_PTE_ADDR_MASK);
            page_pml4e_unmap(pml4,vaddr,block);
            if(page_pt_is_empty(pml4))
            {
                page_page_free(pml4);
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
STATIC UINT64 EFIAPI page_vflags_to_pflags(IN UINT64 flags)
{
    UINT64 pflags=0;

    if(flags&AOS_BOOT_VMA_USER)
    {
        pflags|=PAGE_PTE_US;
    }

    if(flags&AOS_BOOT_VMA_GLOBAL)
    {
        pflags|=PAGE_PTE_G;
    }

    UINT64 rwx=flags&AOS_BOOT_VMA_RWX_MASK;
    if(nx)
    {
        switch(rwx)
        {
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE:
                pflags|=PAGE_PTE_RW;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE:
                pflags|=PAGE_PTE_RW|PAGE_PTE_NX;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_EXECUTE:
                break;
            case AOS_BOOT_VMA_READ:
            default:
                pflags|=PAGE_PTE_NX;
                break;
        }
    }
    else
    {
        switch(rwx)
        {
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE:
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE:
                pflags|=PAGE_PTE_RW;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_EXECUTE:
            case AOS_BOOT_VMA_READ:
            default:
                break;
        }
    }

    switch(flags&AOS_BOOT_VMA_CACHE_MASK)
    {
        case AOS_BOOT_VMA_CACHE_WB:
            break;
        case AOS_BOOT_VMA_CACHE_WP:
            pflags|=PAGE_PDE_PAT|PAGE_PTE_PAT|PAGE_PTE_PWT;
            break;
        case AOS_BOOT_VMA_CACHE_WT:
            pflags|=PAGE_PTE_PWT;
            break;
        case AOS_BOOT_VMA_CACHE_WC:
            pflags|=PAGE_PDE_PAT|PAGE_PTE_PAT;
            break;
        case AOS_BOOT_VMA_CACHE_UCM:
            pflags|=PAGE_PTE_PCD;
            break;
        case AOS_BOOT_VMA_CACHE_UC:
        default:
            pflags|=PAGE_PTE_PWT|PAGE_PTE_PCD;
            break;
    }

    return pflags;
}

/**
 * 将物理地址范围映射到5级分页的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pml5_map(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(pages>EFI_SIZE_TO_PAGES(SIZE_128PB))
    {
        panic(EFI_INVALID_PARAMETER,"Page count exceeds 128PB linear address limit.");
    }

    UINTN vamask=vaddr&PAGE_VADDR5_RESERVED_MASK;
    UINTN vlmask=(vaddr+EFI_PAGES_TO_SIZE(pages)-1)&PAGE_VADDR5_RESERVED_MASK;
    if((vamask!=0&&vamask!=PAGE_VADDR5_RESERVED_MASK)||vamask!=vlmask)
    {
        panic(EFI_INVALID_PARAMETER,"The linear address range is not in canonical form.");
    }

    UINT64 pflags=page_vflags_to_pflags(flags);
    page_pml5e_unmap(pt_root,vaddr,pages);
    page_pml5e_map(pt_root,vaddr,paddr,pages,pflags);
}

/**
 * 将物理地址范围映射到4级分页的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pml4_map(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(pages>EFI_SIZE_TO_PAGES(SIZE_256TB))
    {
        panic(EFI_INVALID_PARAMETER,"Page count exceeds 256TB linear address limit.");
    }

    UINTN vamask=vaddr&PAGE_VADDR4_RESERVED_MASK;
    UINTN vlmask=(vaddr+EFI_PAGES_TO_SIZE(pages)-1)&PAGE_VADDR4_RESERVED_MASK;
    if((vamask!=0&&vamask!=PAGE_VADDR4_RESERVED_MASK)||vamask!=vlmask)
    {
        panic(EFI_INVALID_PARAMETER,"The linear address range is not in canonical form.");
    }

    UINT64 pflags=page_vflags_to_pflags(flags);
    page_pml4e_unmap(pt_root,vaddr,pages);
    page_pml4e_map(pt_root,vaddr,paddr,pages,pflags);
}

/**
 * 解除5级分页内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pml5_unmap(IN UINTN start,IN UINTN end)
{
    if(end<=start)
    {
        panic(EFI_INVALID_PARAMETER,"The address range is empty or inverted.");
    }
    UINTN pages=EFI_SIZE_TO_PAGES(end-start);
    page_pml5e_unmap(pt_root,start,pages);
}

/**
 * 解除4级分页内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI page_pml4_unmap(IN UINTN start,IN UINTN end)
{
    if(end<=start)
    {
        panic(EFI_INVALID_PARAMETER,"The address range is empty or inverted.");
    }
    UINTN pages=EFI_SIZE_TO_PAGES(end-start);
    page_pml4e_unmap(pt_root,start,pages);
}

/**
 * 初始化页表池和页表池位图。
 * 
 * @param parmas 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI init_page_table_pool(IN OUT aos_boot_params* params)
{
    ptbase=(UINTN)allocate_page(AOS_BOOT_MEMORY_PAGE_TABLE_POOL,CONFIG_PAGE_TABLE_POOL,SIZE_4GB-1);
    ptpages=CONFIG_PAGE_TABLE_POOL;
    params->memory.pt_pool_pages=CONFIG_PAGE_TABLE_POOL;

    ptmap_size=CONFIG_PAGE_TABLE_POOL/8+(CONFIG_PAGE_TABLE_POOL%8?1:0);
    ptmap=(UINT8*)pool_alloc(ptmap_size);
    if(ptmap==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate page table pool bitmap from bootstrap pool.");
    }
    if(CONFIG_PAGE_TABLE_POOL%8)
    {
        ptmap[ptmap_size-1]=MAX_UINT8<<(CONFIG_PAGE_TABLE_POOL%8);
    }
    params->memory.pt_pool_map=ptmap;

    nx=params->features.features&AOS_FEATURES_NX;
    page1gb=params->features.features&AOS_FEATURES_PAGE1GB;
    if(params->state.state&AOS_STATE_LA57)
    {
        page_pml_map_handle=page_pml5_map;
        page_pml_unmap_handle=page_pml5_unmap;
    }
    else
    {
        page_pml_map_handle=page_pml4_map;
        page_pml_unmap_handle=page_pml4_unmap;
    }

    pt_root=page_page_alloc();
    params->memory.page_table=(UINTN)pt_root;

    ready=TRUE;

    /*预先映射低4GB，在线性区管理之外*/
    page_pml_map_handle(0,0,EFI_SIZE_TO_PAGES(SIZE_4GB),AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_EXECUTE|
        AOS_BOOT_VMA_CACHE_UC);
}

/**
 * 将物理地址范围映射到页表的线性地址范围。
 * 
 * @param vaddr 线性起始地址。
 * @param paddr 物理起始地址。
 * @param pages 映射页数。
 * @param flags 线性区标志。
 * 
 * @return 无返回值。
 */
VOID EFIAPI page_table_map(IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(!ready)
    {
        panic(EFI_UNSUPPORTED,"The root page table is not initialized.");
    }

    if((paddr&PAGE_4K_OFFSET_MASK)||(vaddr&PAGE_4K_OFFSET_MASK))
    {
        panic(EFI_INVALID_PARAMETER,"The address is not 4K-aligned.");
    }

    if(paddr&PAGE_PADDR_RESERVED_MASK)
    {
        panic(EFI_INVALID_PARAMETER,"The physical address has reserved bits set.");
    }

    if(flags&AOS_BOOT_VMA_HUGEPAGE)
    {
        if((paddr&PAGE_2M_OFFSET_MASK)||(vaddr&PAGE_2M_OFFSET_MASK))
        {
            panic(EFI_INVALID_PARAMETER,"The address is not 2MB-aligned for huge page mapping.");
        }

        if(pages%EFI_SIZE_TO_PAGES(SIZE_2MB))
        {
            panic(EFI_INVALID_PARAMETER,"The page count is not a multiple of the huge page size.");
        }
    }

    UINTN plimit=paddr+EFI_PAGES_TO_SIZE(pages)-1;
    if(pages==0||plimit<paddr)
    {
        panic(EFI_INVALID_PARAMETER,"The address range wraps around.");
    }

    page_pml_map_handle(vaddr,paddr,pages,flags);
}

/**
 * 解除页表内的线性地址范围。
 * 
 * @param start 线性起始地址。
 * @param end   线性结束地址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI page_table_unmap(IN UINTN start,IN UINTN end)
{
    if(!ready)
    {
        panic(EFI_UNSUPPORTED,"The root page table is not initialized.");
    }
    page_pml_unmap_handle(start,end);
}
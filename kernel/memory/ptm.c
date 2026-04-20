/**
 * 内核页表管理系统。
 * @date 2026-03-30
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <memory/vma.h>
#include <support/asm.h>
#include <support/type.h>
#include <support/util.h>

/**
 * 页表项地址掩码。
 */
const uintn PTM_ADDR_MASK=0x000FFFFFFFFFF000ULL;

/**
 * 页表体系高半核映射偏移。
 */
const uintn PTM_PAGE_TABLE_OFFSET=-10*SIZE_512GB;

/**
 * 位图。
 */
static uint64 bitmap[8];

/**
 * 页面缓存池。
 */
static uint64 pool[512];

/**
 * 不可执行位功能状态。
 */
static bool nx_status=false;

/**
 * 五级分页状态。
 */
static bool pml5_status=false;

/**
 * 内核根页表。
 */
static uint64* kernel_pml=null;

/**
 * 检查在4级分页下映射一块线性区域需要多少页来构建页表。
 * 
 * @param pml4   四级分页的PML4页表。
 * @param vaddr  线性区域基址。
 * @param paddr  物理区域基址。
 * @param pages  区域页数。
 * 
 * @return 返回所需页数。
 */
static uintn ptm_calculate_pml4_required_pages(uint64* pml4,uintn vaddr,uintn paddr,uintn pages)
{

}

/**
 * 将VMA的标志组变换成页表标志组。
 * 
 * @param flags VMA标志组。
 * 
 * @return 页表标志组。不合格标志组返回最大值。
 */
static uint64 ptm_vflags_to_pflags(uint64 flags)
{
    uint64 pte=0;

    if(flags&VMA_FLAG_USER)
    {
        /*页表US位*/
        pte|=BIT2;
    }

    if(flags&VMA_FLAG_GLOBAL)
    {
        /*页表G位*/
        pte|=BIT8;
    }

    uint64 rwx=flags&(VMA_FLAG_READ|VMA_FLAG_WRITE|VMA_FLAG_EXECUTE);
    if(nx_status)
    {
        switch(rwx)
        {
            case VMA_FLAG_READ|VMA_FLAG_WRITE|VMA_FLAG_EXECUTE:
            case VMA_FLAG_WRITE|VMA_FLAG_EXECUTE:
                /*页表RW位*/
                pte|=BIT1;
                break;
            case VMA_FLAG_READ|VMA_FLAG_WRITE:
            case VMA_FLAG_WRITE:
                /*页表RW位、NX位*/
                pte|=BIT1|BIT63;
                break;
            case VMA_FLAG_READ|VMA_FLAG_EXECUTE:
            case VMA_FLAG_EXECUTE:
                break;
            case VMA_FLAG_READ:
            default:
                /*页表NX位*/
                pte|=BIT63;
                break;
        }
    }
    else
    {
        switch(rwx)
        {
            case VMA_FLAG_READ|VMA_FLAG_WRITE|VMA_FLAG_EXECUTE:
            case VMA_FLAG_WRITE|VMA_FLAG_EXECUTE:
            case VMA_FLAG_READ|VMA_FLAG_WRITE:
            case VMA_FLAG_WRITE:
                /*页表RW位*/
                pte|=BIT1;
                break;
            case VMA_FLAG_READ|VMA_FLAG_EXECUTE:
            case VMA_FLAG_EXECUTE:
            case VMA_FLAG_READ:
            default:
                break;
        }
    }

    switch(flags&VMA_TYPE_MASK)
    {
        case VMA_TYPE_MEMORY:
            /*WB类型*/
            break;
        case VMA_TYPE_DEVICE:
            /*UC类型，页表PWT、PCD位*/
            pte|=BIT3|BIT4;
            break;
        case VMA_TYPE_READ_OPTIMIZED_DEVICE:
            /*WT类型，页表PWT位*/
            pte|=BIT3;
            break;
        case VMA_TYPE_WRITE_OPTIMIZED_DEVICE:
            /*WC类型，大页PAT、小页PAT位*/
            pte|=BIT12|BIT7;
            break;
        default:
            return UINT64_MAX;
    }

    return pte;
}

/**
 * 返回页表是否为空。
 * 
 * @param pt 页表地址。
 * 
 * @return 为空返回真。
 */
static bool ptm_pt_is_empty(uint64* pt)
{
    uint64 value=0;
    uintn index=0;
    while(index<512)
    {
        value|=pt[index];
        value|=pt[index+1];
        value|=pt[index+2];
        value|=pt[index+3];
        value|=pt[index+4];
        value|=pt[index+5];
        value|=pt[index+6];
        value|=pt[index+7];
        if(value!=0)
        {
            return false;
        }
        index+=8;
    }
    return true;
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
static void ptm_pte_unmap(uint64* pt,uintn vaddr,uintn pages)
{
    uintn index=(vaddr>>12)&0x1FF;
    while(pages>0)
    {
        pt[index]=0;
        index++;
        pages--;
    }
}

/**
 * 将页表内指定线性地址解除映射并清除对应TLB条目。
 * 
 * @param pt    页表地址。
 * @param vaddr 映射起始地址。
 * @param pages 映射页数。
 * 
 * @return 无返回值。
 */
static void ptm_pte_unmap_flush(uint64* pt,uintn vaddr,uintn pages)
{
    uintn index=(vaddr>>12)&0x1FF;
    while(pages>0)
    {
        pt[index]=0;
        flush_single_tlb(vaddr);
        index++;
        vaddr+=SIZE_4KB;
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
static void ptm_pde_unmap(uint64* pd,uintn vaddr,uintn pages)
{
    while(pages>0)
    {
        uintn index=(vaddr>>21)&0x1FF;
        uintn block=0x200-((vaddr>>12)&0x1FF);
        block=min(block,pages);
        uint64 pde=pd[index];
        if(pde&BIT0)
        {
            /*大页PS位*/
            if(pde&BIT7)
            {
                pd[index]=0;
            }
            else
            {
                uint64* pt=(uint64*)((pde&PTM_ADDR_MASK)+PTM_PAGE_TABLE_OFFSET);
                ptm_pte_unmap(pt,vaddr,block);
                if(ptm_pt_is_empty(pt))
                {
                    ptm_page_free(pt);
                    pd[index]=0;
                }
            }
        }
        pages-=block;
        vaddr+=block<<12;
    }
}
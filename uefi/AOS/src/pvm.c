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
STATIC UINT64* EFIAPI pvm_page_alloc()
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
        UINT64* addr=(UINT64*)((index<<12)+pbase);
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
        pflags|=BIT2;
    }

    if(flags&AOS_BOOT_VMA_GLOBAL)
    {
        pflags|=BIT8;
    }

    UINT64 rwx=flags&AOS_BOOT_VMA_RWX_MASK;
    if(nx)
    {
        switch(rwx)
        {
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE:
                pflags|=BIT1|BIT63;
                break;
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_EXECUTE:
                break;
            case AOS_BOOT_VMA_READ:
            default:
                pflags|=BIT63;
                break;
        }
    }
    else
    {
        switch(rwx)
        {
            case AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE:
                pflags|=BIT1;
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
            pflags|=BIT12|BIT7|BIT3;
            break;
        case AOS_BOOT_VMA_TYPE_WT:
            pflags|=BIT3;
            break;
        case AOS_BOOT_VMA_TYPE_WC:
            pflags|=BIT12|BIT7;
            break;
        case AOS_BOOT_VMA_TYPE_UCM:
            pflags|=BIT4;
            break;
        case AOS_BOOT_VMA_TYPE_UC:
        default:
            pflags|=BIT3|BIT4;
            break;
    }

    return pflags;
}

/* 
 * 将物理内存块映射到页表内。仅映射，不关心是什么状态下的映射。其后续存在性维护不归映射函数管理。
 * 
 * @param pml   顶级页表地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI (*pvm_pt_map)(IN VOID* pml,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags);

/* 
 * 将物理内存块映射到页表项内。
 * 
 * @param pt    页表地址。
 * @param vaddr 映射目标起始地址。
 * @param paddr 映射物理起始地址。
 * @param pages 内存块页数。
 * @param flags 映射标志。
 * 
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI pvm_pte_map(IN VOID* pt,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    
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
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI pvm_pde_map(IN VOID* pd,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{

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
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI pvm_pdpe_map(IN VOID* pdp,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{

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
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI pvm_pml4e_map(IN VOID* pml4,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{

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
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI pvm_pml5e_map(IN VOID* pml5,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{

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
    return EFI_SUCCESS;
}
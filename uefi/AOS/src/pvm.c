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
STATIC VOID EFIAPI pvm_add_vma(IN OUT aos_boot_params* params,IN aos_vma* vma)
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
STATIC VOID EFIAPI pvm_remove_vma(IN OUT aos_boot_params* params,IN aos_vma* vma)
{
    if(params->vma_head!=NULL)
    {
        aos_vma* node=params->vma_head;
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
STATIC EFI_STATUS EFIAPI (*pvm_pt_map)(IN VOID* pml,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT32 flags);

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
STATIC EFI_STATUS EFIAPI pvm_pte_map(IN VOID* pt,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT32 flags)
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
STATIC EFI_STATUS EFIAPI pvm_pde_map(IN VOID* pd,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT32 flags)
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
STATIC EFI_STATUS EFIAPI pvm_pdpe_map(IN VOID* pdp,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT32 flags)
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
STATIC EFI_STATUS EFIAPI pvm_pml4e_map(IN VOID* pml4,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT32 flags)
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
STATIC EFI_STATUS EFIAPI pvm_pml5e_map(IN VOID* pml5,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT32 flags)
{

}

/* 
 * 构造内核线性区描述。
 * 
 * @param params 启动参数。
 * 
 * @return 一般成功，不应该有资源不足问题。
 */
STATIC EFI_STATUS EFIAPI pvm_create_vma(IN OUT aos_boot_params* params)
{
    aos_vma* node=(aos_vma*)malloc(sizeof(aos_vma));
    if(node==NULL)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Failed to allocate AOS VMA.\n"));
        return EFI_OUT_OF_RESOURCES;
    }

    node->start=(UINTN)params->gdt_base;
    node->end=EFI_PAGES_TO_SIZE(EFI_SIZE_TO_PAGES(params->gdt_size))+node->start;
    node->flags=AOS_VMA_READ;
    pvm_add_vma(params,node);

    node=(aos_vma*)malloc(sizeof(aos_vma));
    if(node==NULL)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.pvm] Failed to allocate AOS VMA.\n"));
        return EFI_OUT_OF_RESOURCES;
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
    return EFI_SUCCESS;
}
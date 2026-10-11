/**
 * 模块线性区管理。
 * @date 2026-08-07
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Guid/MemoryAttributesTable.h>
#include <Library/UefiLib.h>

#include "page.h"
#include "panic.h"
#include "params.h"
#include "pool.h"

/**
 * 线性区链表头。
 */
STATIC aos_boot_vma* head=NULL;

/**
 * 线性区链表尾。
 */
STATIC aos_boot_vma* tail=NULL;


/**
 * 添加线性区到线性区链表。
 * 
 * @param vma   线性区。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI vma_add_node(IN aos_boot_vma* vma)
{
    vma->prev=NULL;
    vma->next=NULL;

    if(head==NULL)
    {
        head=vma;
        tail=vma;
    }
    else
    {
        aos_boot_vma* pnode=NULL;
        aos_boot_vma* cnode=head;
        while(cnode!=NULL&&cnode->end<=vma->start)
        {
            pnode=cnode;
            cnode=cnode->next;
        }

        if(pnode==NULL)
        {
            vma->next=cnode;
            cnode->prev=vma;
            head=vma;
        }
        else if(cnode==NULL)
        {
            pnode->next=vma;
            vma->prev=pnode;
            tail=vma;
        }
        else
        {
            vma->next=cnode;
            cnode->prev=vma;
            pnode->next=vma;
            vma->prev=pnode;
        }
    }
}

/**
 * 在线性区链表删除一个线性区。删除依据以指针一致为准。
 * 
 * @param vma   线性区。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI vma_remove_node(IN aos_boot_vma* vma)
{
    if(head!=NULL)
    {
        aos_boot_vma* node=head;
        aos_boot_vma* pnode=NULL;
        while(node!=NULL&&node!=vma)
        {
            pnode=node;
            node=node->next;
        }

        if(node==NULL)
        {
            return;
        }

        if(pnode==NULL)
        {
            head=node->next;
            if(head!=NULL)
            {
                head->prev=NULL;
            }
            else
            {
                tail=NULL;
            }
        }
        else
        {
            pnode->next=node->next;
            if(pnode->next!=NULL)
            {
                pnode->next->prev=pnode;
            }
            else
            {
                tail=pnode;
            }
        }
        node->next=NULL;
        node->prev=NULL;
    }
}

/**
 * 通过线性地址寻找对应的线性区结点。
 * 
 * @param vaddr 线性区域地址。
 * 
 * @return 找到返回结点地址，未找到返回空。
 */
STATIC aos_boot_vma* EFIAPI vma_find_node(IN UINTN vaddr)
{
    aos_boot_vma* node=head;
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
STATIC BOOLEAN EFIAPI vma_check_node(IN UINTN vaddr,IN UINTN pages)
{
    aos_boot_vma* node=head;
    UINTN start,end;

    UINTN limit=vaddr+EFI_PAGES_TO_SIZE(pages)-1;
    if(limit<vaddr)
    {
        panic(EFI_INVALID_PARAMETER,"The VMA address range wraps around.");
    }

    while(node!=NULL)
    {
        start=node->start;
        end=node->end-1;

        if(start>limit)
        {
            return FALSE;
        }
        else if(vaddr<=end)
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
 * 添加一个线性区。
 * 
 * @param type  线性区类型。
 * @param vaddr 线性区域基址。
 * @param paddr 物理区域基址。
 * @param pages 区域页数。
 * @param flags 线性区标记。
 * 
 * @return 无返回值。
 */
VOID EFIAPI add_vma(IN UINT64 type,IN UINTN vaddr,IN UINTN paddr,IN UINTN pages,IN UINT64 flags)
{
    if(pages==0)
    {
        panic(EFI_INVALID_PARAMETER,"The page count is zero.");
    }

    if(vma_check_node(vaddr,pages))
    {
        panic(EFI_UNSUPPORTED,"The VMA overlaps with an existing VMA.");
    }

    aos_boot_vma* vma=pool_alloc(sizeof(aos_boot_vma));
    if(vma==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate VMA node from bootstrap pool.");
    }

    page_table_map(vaddr,paddr,pages,flags);
    vma->start=vaddr;
    vma->end=vaddr+EFI_PAGES_TO_SIZE(pages);
    vma->flags=flags|AOS_BOOT_VMA_ALLOCATED;
    vma->type=type;
    vma_add_node(vma);
}

/**
 * 删除一个线性区。仅在通过输入参数能够找到线性区时才会删除。
 * 
 * @param vaddr 线性区域地址。
 * 
 * @return 无返回值。
 */
VOID EFIAPI remove_vma(IN UINTN vaddr)
{
    aos_boot_vma* node=vma_find_node(vaddr);
    if(node!=NULL)
    {
        page_table_unmap(node->start,node->end);
        vma_remove_node(node);
        pool_free(node);
    }
}

/**
 * 将EFI内存属性转换成VMA标志。
 * 
 * @param attrs 描述符属性。
 * @param mat   MAT属性。
 * 
 * @return VMA标志。
 */
STATIC UINT64 EFIAPI vma_attrs_to_flags(IN UINT64 attrs,IN UINT64 mat)
{
    UINT64 flags=AOS_BOOT_VMA_READ;

    if(attrs&EFI_MEMORY_WB)
    {
        flags|=AOS_BOOT_VMA_CACHE_WB;
    }
    else
    {
        flags|=AOS_BOOT_VMA_CACHE_UC;
    }

    if(!(mat&EFI_MEMORY_RO))
    {
        flags|=AOS_BOOT_VMA_WRITE;
    }

    if(!(mat&EFI_MEMORY_XP))
    {
        flags|=AOS_BOOT_VMA_EXECUTE;
    }

    return flags;
}

/**
 * 添加一个运行时属性线性区。
 * 
 * @param runtime 运行时属性描述符。
 * @param mat     内存属性表。
 * @param base    线性区偏移基址。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI vma_set_runtime_vma(IN OUT EFI_MEMORY_DESCRIPTOR* runtime,IN EFI_MEMORY_ATTRIBUTES_TABLE* mat,
    IN UINTN base)
{
    runtime->VirtualStart=base+runtime->PhysicalStart;
    if(mat!=NULL&&(runtime->Type==EfiRuntimeServicesCode||runtime->Type==EfiRuntimeServicesData))
    {
        EFI_MEMORY_DESCRIPTOR* mat_map=(EFI_MEMORY_DESCRIPTOR*)((UINTN)mat+sizeof(EFI_MEMORY_ATTRIBUTES_TABLE));
        UINTN bl=runtime->PhysicalStart;
        UINTN br=runtime->PhysicalStart+EFI_PAGES_TO_SIZE(runtime->NumberOfPages)-1;
        UINTN amount=0;

        for(UINTN i=0;i<mat->NumberOfEntries;i++)
        {
            EFI_MEMORY_DESCRIPTOR* a=(EFI_MEMORY_DESCRIPTOR*)((UINTN)mat_map+i*mat->DescriptorSize);
            UINTN al=a->PhysicalStart;
            UINTN ar=a->PhysicalStart+EFI_PAGES_TO_SIZE(a->NumberOfPages)-1;
            
            if(bl<=al&&br>=ar)
            {
                amount+=a->NumberOfPages;
            }
        }

        if(amount==runtime->NumberOfPages)
        {
            for(UINTN i=0;i<mat->NumberOfEntries;i++)
            {
                EFI_MEMORY_DESCRIPTOR* a=(EFI_MEMORY_DESCRIPTOR*)((UINTN)mat_map+i*mat->DescriptorSize);
                UINTN al=a->PhysicalStart;
                UINTN ar=a->PhysicalStart+EFI_PAGES_TO_SIZE(a->NumberOfPages)-1;

                if(bl<=al&&br>=ar)
                {
                    add_vma(AOS_BOOT_VMA_TYPE_FIRMWARE,a->PhysicalStart+base,a->PhysicalStart,
                        a->NumberOfPages,vma_attrs_to_flags(runtime->Attribute,a->Attribute));
                }
            }
            return;
        }
    }

    add_vma(AOS_BOOT_VMA_TYPE_FIRMWARE,runtime->VirtualStart,runtime->PhysicalStart,
        runtime->NumberOfPages,vma_attrs_to_flags(runtime->Attribute,runtime->Attribute));
}

/**
 * 用EFI内存图和内存属性表填充AOS线性区，并为EFI内存图赋值。
 * 
 * @param base      固件线性基址。
 * @param map       EFI内存图。
 * @param map_size  内存图大小。
 * @param desc_size 描述符大小。
 * 
 * @return 已完成的线性区头节点。
 */
aos_boot_vma* EFIAPI fill_firmware_vma(IN UINTN base,IN OUT EFI_MEMORY_DESCRIPTOR* map,IN UINTN map_size,
    IN UINTN desc_size)
{
    if(map==NULL||map_size==0||desc_size<sizeof(EFI_MEMORY_DESCRIPTOR))
    {
        panic(EFI_INVALID_PARAMETER,"Invalid EFI memory map parameters.");
    }

    EFI_MEMORY_ATTRIBUTES_TABLE* mat=NULL;
    EFI_STATUS status=EfiGetSystemConfigurationTable(&gEfiMemoryAttributesTableGuid,(VOID**)&mat);
    if(status==EFI_SUCCESS&&mat!=NULL&&mat->NumberOfEntries>0)
    {
        EFI_MEMORY_DESCRIPTOR* mat_map=(EFI_MEMORY_DESCRIPTOR*)((UINTN)mat+sizeof(EFI_MEMORY_ATTRIBUTES_TABLE));

        for(UINTN i=0;mat!=NULL&&i<mat->NumberOfEntries;i++)
        {
            EFI_MEMORY_DESCRIPTOR* a=(EFI_MEMORY_DESCRIPTOR*)((UINTN)mat_map+i*mat->DescriptorSize);
            UINTN al=a->PhysicalStart;
            UINTN ar=a->PhysicalStart+EFI_PAGES_TO_SIZE(a->NumberOfPages)-1;
            for(UINTN j=i+1;j<mat->NumberOfEntries;j++)
            {
                EFI_MEMORY_DESCRIPTOR* b=(EFI_MEMORY_DESCRIPTOR*)((UINTN)mat_map+j*mat->DescriptorSize);
                UINTN bl=b->PhysicalStart;
                UINTN br=b->PhysicalStart+EFI_PAGES_TO_SIZE(b->NumberOfPages)-1;
                if(al<=br&&bl<=ar)
                {
                    mat=NULL;
                    break;
                }
            }
        }
    }
    else
    {
        mat=NULL;
    }

    EFI_MEMORY_DESCRIPTOR* node=map;
    UINTN end=map_size+(UINTN)map;
    while((UINTN)node<end)
    {
        if(node->Attribute&EFI_MEMORY_RUNTIME)
        {
            vma_set_runtime_vma(node,mat,base);
        }
        node=(EFI_MEMORY_DESCRIPTOR*)((UINTN)node+desc_size);
    }

    return head;
}
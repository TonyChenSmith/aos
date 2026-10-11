/**
 * 模块内存标记管理。
 * @date 2026-08-21
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Library/UefiBootServicesTableLib.h>

#include "config.h"
#include "panic.h"
#include "params.h"
#include "pool.h"

/**
 * 内存图头结点。
 */
STATIC aos_boot_memory* head=NULL;

/**
 * 内存图尾结点。
 */
STATIC aos_boot_memory* tail=NULL;

/**
 * 按起始地址升序插入结点，并验证无重复。
 * 
 * @param node 待添加结点。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI mark_add_node(IN aos_boot_memory* node)
{
    aos_boot_memory* current=tail;
    while(current!=NULL&&current->start>=node->start)
    {
        current=current->prev;
    }

    node->prev=current;
    node->next=current?current->next:head;

    if(node->prev&&node->prev->start+node->prev->size>node->start)
    {
        panic(EFI_DEVICE_ERROR,"Memory map overlap detected.");
    }

    if(node->next&&node->start+node->size>node->next->start)
    {
        panic(EFI_DEVICE_ERROR,"Memory map overlap detected.");
    }

    if(node->prev)
    {
        node->prev->next=node;
    }
    else
    {
        head=node;
    }

    if(node->next)
    {
        node->next->prev=node;
    }
    else
    {
        tail=node;
    }
}

/**
 * 页面申请并标记。
 * 
 * @param type  申请页面类型。
 * @param pages 申请页数。
 * @param max   申请的最大边界。
 * 
 * @return 申请页面的起始地址。
 */
VOID* EFIAPI allocate_page(IN UINT64 type,IN UINTN pages,IN UINTN max)
{
    if(pages==0)
    {
        panic(EFI_INVALID_PARAMETER,"The page count is zero.");
    }
    EFI_PHYSICAL_ADDRESS addr=max;
    panic_if_error(gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,pages,&addr),
            "Failed to allocate memory.");
    aos_boot_memory* node=pool_alloc(sizeof(aos_boot_memory));
    if(node==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
    }
    node->start=(UINTN)addr;
    node->size=EFI_PAGES_TO_SIZE(pages);
    node->type=type;
    node->prev=NULL;
    node->next=NULL;
    mark_add_node(node);
    return (VOID*)(UINTN)addr;
}

/**
 * 将引导器数据切割出自由内存。
 * 
 * @param start 起始地址。
 * @param size  内存块大小。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI mark_loader_data_to_free(IN UINTN start,IN UINTN size)
{
    UINTN end=start+size;
    UINTN current=start;

    if(size==0||end<=start)
    {
        panic(EFI_INVALID_PARAMETER,"The loader data range is invalid.");
    }

    if(start&EFI_PAGE_MASK||size&EFI_PAGE_MASK)
    {
        panic(EFI_INVALID_PARAMETER,"The loader data is not 4K-aligned.");
    }

    aos_boot_memory* node=head;
    while(node!=NULL&&node->start+node->size<=current)
    {
        node=node->next;
    }

    /*前空隙*/
    while(node!=NULL&&node->start<end)
    {
        if(current<node->start)
        {
            UINTN gap=node->start-current;
            if(gap<EFI_PAGE_SIZE)
            {
                panic(EFI_DEVICE_ERROR,"Invalid loader data gap detected.");
            }

            aos_boot_memory* free_node=pool_alloc(sizeof(aos_boot_memory));
            if(free_node==NULL)
            {
                panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
            }
            free_node->start=current;
            free_node->size=gap;
            free_node->type=AOS_BOOT_MEMORY_FREE;
            free_node->prev=NULL;
            free_node->next=NULL;
            mark_add_node(free_node);
        }

        current=MIN(node->start+node->size,end);
        if(current<end)
        {
            node=node->next;
        }
        else
        {
            break;
        }
    }

    /*尾空隙*/
    if(current<end)
    {
        aos_boot_memory* free_node=pool_alloc(sizeof(aos_boot_memory));
        if(free_node==NULL)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
        }
        free_node->start=current;
        free_node->size=end-current;
        free_node->type=AOS_BOOT_MEMORY_FREE;
        free_node->prev=NULL;
        free_node->next=NULL;
        mark_add_node(free_node);
    }
}

/**
 * 通过EFI内存描述符映射为AOS内存类型。
 * 
 * @param desc 内存描述符。
 * 
 * @return AOS内存类型。
 */
STATIC UINT64 EFIAPI mark_desc_type(IN EFI_MEMORY_DESCRIPTOR* desc)
{
    if(desc->Attribute&EFI_MEMORY_RUNTIME)
    {
        return AOS_BOOT_MEMORY_FIRMWARE;
    }
    switch(desc->Type)
    {
        case EfiLoaderCode:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
            if(desc->PhysicalStart<SIZE_1MB)
            {
                return AOS_BOOT_MEMORY_1MB;
            }
            else
            {
                return AOS_BOOT_MEMORY_FREE;
            }
        case EfiACPIReclaimMemory:
            return AOS_BOOT_MEMORY_ACPI_TABLE;
        case EfiACPIMemoryNVS:
            return AOS_BOOT_MEMORY_ACPI_NVS;
        case EfiPersistentMemory:
            return AOS_BOOT_MEMORY_PERSISTENT;
        default:
            return AOS_BOOT_MEMORY_RESERVED;
    }
}

/**
 * 填充一块内存描述符。
 * 
 * @param desc 内存描述符。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI mark_desc_to_map(IN EFI_MEMORY_DESCRIPTOR* desc)
{
    if(desc->Type==EfiLoaderData)
    {
        return;
    }
    UINT64 type=mark_desc_type(desc);
    if(type==AOS_BOOT_MEMORY_1MB)
    {
        aos_boot_memory* node1=pool_alloc(sizeof(aos_boot_memory));
        if(node1==NULL)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
        }
        node1->type=AOS_BOOT_MEMORY_1MB;
        node1->start=desc->PhysicalStart;
        node1->size=MIN(desc->PhysicalStart+EFI_PAGES_TO_SIZE(desc->NumberOfPages),SIZE_1MB)-desc->PhysicalStart;
        node1->prev=NULL;
        node1->next=NULL;
        mark_add_node(node1);
        if(EFI_SIZE_TO_PAGES(node1->size)<desc->NumberOfPages)
        {
            aos_boot_memory* node2=pool_alloc(sizeof(aos_boot_memory));
            if(node2==NULL)
            {
                panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
            }
            node2->type=AOS_BOOT_MEMORY_FREE;
            node2->start=node1->start+node1->size;
            node2->size=EFI_PAGES_TO_SIZE(desc->NumberOfPages)-node1->size;
            node2->prev=NULL;
            node2->next=NULL;
            mark_add_node(node2);
        }
    }
    else
    {
        aos_boot_memory* node=pool_alloc(sizeof(aos_boot_memory));
        if(node==NULL)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
        }
        node->type=type;
        node->start=desc->PhysicalStart;
        node->size=EFI_PAGES_TO_SIZE(desc->NumberOfPages);
        node->prev=NULL;
        node->next=NULL;
        mark_add_node(node);
    }
}

/**
 * 合并相邻同类型结点。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI mark_merge_node(VOID)
{
    aos_boot_memory* node=head;
    while(node!=NULL&&node->next!=NULL)
    {
        if(node->type==node->next->type&&node->start+node->size==node->next->start)
        {
            aos_boot_memory* dead=node->next;
            node->size+=dead->size;
            node->next=dead->next;
            if(dead->next!=NULL)
            {
                dead->next->prev=node;
            }
            else
            {
                tail=node;
            }
            pool_free(dead);
        }
        else
        {
            node=node->next;
        }
    }
}

/**
 * 用EFI内存图填充AOS内存图。
 * 
 * @param map       EFI内存图。
 * @param map_size  内存图大小。
 * @param desc_size 描述符大小。
 * @param boot_pool 引导内存池基址。
 * 
 * @return 已填充完成的内存图头结点。
 */
aos_boot_memory* EFIAPI fill_memory_map(IN EFI_MEMORY_DESCRIPTOR* map,IN UINTN map_size,IN UINTN desc_size,
    IN UINTN boot_pool)
{
    aos_boot_memory* bpnode=pool_alloc(sizeof(aos_boot_memory));
    if(bpnode==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory map node from bootstrap pool.");
    }
    bpnode->start=boot_pool;
    bpnode->size=EFI_PAGES_TO_SIZE(CONFIG_BOOTSTRAP_POOL);
    bpnode->type=AOS_BOOT_MEMORY_BOOTSTRAP_POOL;
    bpnode->prev=NULL;
    bpnode->next=NULL;
    mark_add_node(bpnode);

    EFI_MEMORY_DESCRIPTOR* desc=map;
    UINTN end=(UINTN)map+map_size;
    while((UINTN)desc<end)
    {
        if(desc->Type==EfiLoaderData)
        {
            mark_loader_data_to_free(desc->PhysicalStart,EFI_PAGES_TO_SIZE(desc->NumberOfPages));
        }
        desc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)desc+desc_size);
    }

    desc=map;
    while((UINTN)desc<end)
    {
        mark_desc_to_map(desc);
        desc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)desc+desc_size);
    }

    mark_merge_node();

    return head;
}
/**
 * 内核伙伴系统。
 * @date 2026-03-21
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <memory/pool.h>
#include <support/util.h>

/**
 * EFI内存类型。
 */
typedef enum _efi_memory_type
{
    EFI_RESERVED_MEMORY_TYPE,                /*保留内存。*/
    EFI_LOADER_CODE,                         /*加载器代码。*/
    EFI_LOADER_DATA,                         /*加载器数据。*/
    EFI_BOOT_SERVICES_CODE,                  /*引导服务代码。*/
    EFI_BOOT_SERVICES_DATA,                  /*引导服务数据。*/
    EFI_RUNTIME_SERVICES_CODE,               /*运行时服务代码。*/
    EFI_RUNTIME_SERVICES_DATA,               /*运行时服务数据。*/
    EFI_CONVENTIONAL_MEMORY,                 /*未分配内存。*/
    EFI_UNUSABLE_MEMORY,                     /*不可用内存。*/
    EFI_ACPI_RECLAIM_MEMORY,                 /*ACPI表内存。*/
    EFI_ACPI_MEMORY_NVS,                     /*ACPI固件保留内存。*/
    EFI_MEMORY_MAPPED_IO,                    /*内存映射输入输出。*/
    EFI_MEMORY_MAPPED_IO_PORT_SPACE,         /*内存映射输入输出。*/
    EFI_PAL_CODE,                            /*固件保留处理器代码。*/
    EFI_PERSISTENT_MEMORY,                   /*未分配非易失性内存。*/
    EFI_UNACCEPTED_MEMORY_TYPE,              /*未接受内存。*/
    EFI_MAX_MEMORY_TYPE,                     /*最大内存类型。*/
    MEMORY_TYPE_OEM_RESERVED_MIN=0X70000000, /*OEM保留最小值。*/
    MEMORY_TYPE_OEM_RESERVED_MAX=0X7FFFFFFF, /*OEM保留最大值。*/
    MEMORY_TYPE_OS_RESERVED_MIN=0X80000000,  /*OS保留最小值。*/
    MEMORY_TYPE_OS_RESERVED_MAX=0XFFFFFFFF   /*OS保留最大值。*/
} efi_memory_type;

/**
 * 低区与高区的内存边界。
 */
const static uintn BUDDY_MEMORY_BOUNDARY=SIZE_4GB;

/**
 * 伙伴系统结点。
 */
typedef struct _buddy_node buddy_node;

struct _buddy_node
{
    buddy_node* next; /*下一结点。*/
    uintn       base; /*页框基址。*/
};

/**
 * 伙伴系统元数据。
 */
typedef struct _buddy_meta
{
    uint64      low_bitmap;  /*低区位图。*/
    uint64      high_bitmap; /*高区位图。*/
    buddy_node* low[52];     /*低区空闲链表。*/
    buddy_node* high[52];    /*高区空闲链表。*/
} buddy_meta;

/**
 * 伙伴系统结点池。
 */
typedef struct _buddy_node_pool buddy_node_pool;

struct _buddy_node_pool
{
    buddy_node_pool* next;       /*下一池。*/
    uint32           count;      /*空闲结点数量。*/
    uint32           first;      /*第一个空闲结点索引。*/
    uint8            bitmap[32]; /*位图。*/
    buddy_node       node[253];  /*结点。*/
};

/**
 * 当前伙伴系统。
 */
static buddy_meta buddy;

/**
 * 当前初始化状态。
 */
static bool init_state=false;

/**
 * 将结点推入到伙伴系统栈。
 * 
 * @param head 链表头。
 * @param node 链表结点。
 * 
 * @return 无返回值。
 */
static inline void buddy_list_push(buddy_node** head,buddy_node* node)
{
    if(*head!=null)
    {
        node->next=*head;
    }
    else
    {
        node->next=null;
    }
    *head=node;
}

/**
 * 将结点从伙伴系统链表中推出。
 * 
 * @param head 链表头。
 * 
 * @return 一个结点。
 */
static inline buddy_node* buddy_list_pop(buddy_node** head)
{
    buddy_node* result=*head;
    if(*head!=null)
    {
        *head=result->next;
    }
    return result;
}

/**
 * 将一块连续页框切割并放回伙伴系统空闲链表。
 * 
 * @param base  页框基址。
 * @param pages 页框页数。
 * 
 * @return 成功返回0，未成功返回未放回页数。
 */
static uintn buddy_spilt(uintn base,uintn pages)
{
    while(pages>0)
    {
        uintn exp=count_trailing_zeros(base)-12;
        uintn block=1<<exp;
        while(block>pages)
        {
            exp--;
            block=1<<exp;
        }
        buddy_node* node=pool_alloc(sizeof(buddy_node));
        if(node==null)
        {
            break;
        }
        node->base=base;
        buddy_list_push(base>=BUDDY_MEMORY_BOUNDARY?&buddy.high[exp]:&buddy.low[exp],node);
        base+=block<<12;
        pages-=block;
    }
    return pages;
}

/**
 * 初始化伙伴系统。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
bool memory_buddy_init(aos_boot_params* params)
{
    return false;
}

/**
 * 从伙伴系统申请页面。
 * 
 * @param pages 页数。
 * 
 * @return 申请成功返回基址，申请失败返回最大值。
 */
uintn buddy_allow(uintn pages)
{
    /*直接计算出最近2的幂指数*/
    uintn start=sizeof(uintn)*8-count_leading_zeros(pages-1);
    if(start>=52)
    {
        return UINTN_MAX;
    }

    uintn index=count_trailing_zeros(buddy.high_bitmap&(UINT64_MAX<<start));
    if(index==64)
    {

    }
    else
    {
        uintn block=1<<index;
        uintn result=
    }
}
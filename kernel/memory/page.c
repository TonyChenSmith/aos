/**
 * 内核每CPU页池。
 * @date 2026-03-09
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/type.h>
#include <support/util.h>

/**
 * 每CPU页池。
 */
typedef struct _per_cpu_page_pool per_cpu_page_pool;

struct _per_cpu_page_pool
{
    per_cpu_page_pool* prev;       /*前一页池。*/
    per_cpu_page_pool* next;       /*后一页池。*/
    uint16             free;       /*池内空闲页数。*/
    uint16             first;      /*池内第一个空闲页。*/
    uint16             last;       /*池内最后一个空闲页。*/
    uint16             magic;      /*魔数。*/
    uintn              paddr[509]; /*物理页面数组。*/
};

/**
 * 每CPU页池魔数。
 */
const uint16 PER_CPU_PAGE_POOL_MAGIC=(((uint16)'C'<<8)|'P');

/**
 * 内核的每CPU页池。
 */
static per_cpu_page_pool* pool=null;

/**
 * 是否完成每CPU池初始化。
 */
static bool finish_init=false;

/**
 * 初始化BSP的每CPU页池。
 * 
 * @param meta  元数据存储区。
 * @param base  添加物理页面基址。
 * @param pages 添加页数。
 * 
 * @return 实际添加页数。
 */
uint16 init_bsp_pcp_pool(void* meta,uintn base,uint16 pages)
{
    if(finish_init)
    {
        return 0;
    }

    pool=(per_cpu_page_pool*)meta;
    pool->magic=PER_CPU_PAGE_POOL_MAGIC;
    pool->first=0;
    
    /*第一次初始化，最多添加300*/
    uint16 result=0;
    pages=min(pages,300);
    for(;result<pages;result++)
    {
        pool->paddr[result]=base+result*0x1000;
    }
    pool->last=result!=0?result-1:0;
    pool->free=result;
    finish_init=true;

    return result;
}
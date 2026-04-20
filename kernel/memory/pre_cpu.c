/**
 * 内核每CPU页池。
 * @date 2026-03-09
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/util.h>

/**
 * 每CPU池位图。
 */
static uint64 bitmap[8];

/**
 * 每CPU池。固定容量。
 */
static uintn pool[512];

/**
 * 当前池内页面总数。
 */
static uintn total=0;

/**
 * 重新平衡每CPU池到一个阈值。
 * 
 * @return 成功填充就返回真。
 */
bool per_cpu_rebalance(void)
{
    /*需要伙伴系统支持*/
    return false;
}

/**
 * 初始化每CPU池。
 * 
 * @return 初始化成功返回真。
 */
bool memory_pre_cpu_init(void)
{
    uintn index=0;
    while(index<512)
    {
        pool[index]=0;
        pool[index+1]=0;
        pool[index+2]=0;
        pool[index+3]=0;
        pool[index+4]=0;
        pool[index+5]=0;
        pool[index+6]=0;
        pool[index+7]=0;
        index+=8;
    }
    bitmap[0]=0;
    bitmap[1]=0;
    bitmap[2]=0;
    bitmap[3]=0;
    bitmap[4]=0;
    bitmap[5]=0;
    bitmap[6]=0;
    bitmap[7]=0;

    return true;
}

/**
 * 向每CPU池申请一块4kB物理页。
 * 
 * @return 有物理页返回页面基址，物理内存消耗到阈值返回0。
 */
uintn per_cpu_allow(void)
{
    if(total==0&&!per_cpu_rebalance())
    {
        return 0;
    }

    uintn index=0;
    uintn n=0;
    for(;n<8;n++)
    {
        index=count_trailing_zeros(bitmap[n]);
        if(index<64)
        {
            break;
        }
    }

    uintn result=pool[index+(n<<6)];
    pool[index+(n<<6)]=0;
    bitmap[n]&=~(((uint64)1)<<index);
    total--;

    return result;
}

/**
 * 向每CPU池释放一块4kB物理页。
 * 
 * @param addr 要释放的页面基址。
 * 
 * @return 无返回值。
 */
void per_cpu_free(uintn addr)
{
    if(addr==0)
    {
        return;
    }

    uintn n=0;
    for(;n<8;n++)
    {
        if(bitmap[n]!=UINT64_MAX)
        {
            break;
        }
    }

    if(n<8)
    {
        uintn index=count_trailing_zeros(~bitmap[n]);;
        pool[(n<<6)+index]=addr;
        bitmap[n]|=((uint64)1)<<index;
        total++;
    }
    else
    {
        /*应该直接释放到伙伴系统*/
    }

    if(total>500)
    {
        per_cpu_rebalance();
    }
}
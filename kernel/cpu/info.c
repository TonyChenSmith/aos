/**
 * 内核CPU信息。
 * @date 2026-03-10
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/asm.h>
#include <support/const.h>

/**
 * MSR IA32_APIC_BASE的基址。
 */
const uint32 IA32_APIC_BASE=0x1B;

/**
 * MSR IA32_X2APIC_APICID的基址。
 */
const uint32 IA32_X2APIC_APICID=0x802;

/**
 * 获取当前运行CPU的编号。
 * 
 * @return 当前CPU编号。
 */
uint32 get_current_cpu_id(void)
{
    uint64 base=read_msr(IA32_APIC_BASE);
    if(!(base&BIT11))
    {
        return 0;
    }
    else if(base&BIT10)
    {
        return (uint32)read_msr(IA32_X2APIC_APICID);
    }
    else
    {
        base&=0xFFFFFFFFFFFFF000UL;
        uint32* id=(uint32*)(base+0x20);
        return *id>>24;
    }
}

/**
 * 获取当前运行CPU的是否是引导处理器。
 * 
 * @return 如果是BSP返回真。
 */
bool is_bootstrap_processor(void)
{
    return read_msr(IA32_APIC_BASE)&BIT8;
}
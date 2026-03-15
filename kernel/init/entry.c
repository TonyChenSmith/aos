/**
 * 内核模块入口。
 * @date 2026-01-26
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <support/asm.h>

extern void print_bytes(const char8* buf,uint64 size);
extern void sleep();

void line(const char8* str)
{
    uintn size=0;
    while(str[size]!=0)
    {
        size++;
    }
    print_bytes(str,size);
    print_bytes("\n",1);
}

/**
 * 重置所有AP。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
static void init_reset_all_aps(aos_boot_params* params)
{
    if(params->state.apic==AOS_APIC_NO_APIC)
    {
        return;
    }
    else if(params->state.apic==AOS_APIC_XAPIC)
    {
        uint64 apic_base=read_msr(0x1B)&0xFFFFFFFFFFFFF000UL;
        uint32* icr_low=(uint32*)(apic_base+0x300);
        uint32* icr_high=(uint32*)(apic_base+0x310);

        *icr_high=0;
        uint32 command=(5<<8)|BIT14|(3<<18);
        *icr_low=command;
        memory_fence();
    }
    else
    {
        write_msr(0x830,(5<<8)|BIT14|(3<<18));
    }
}

/**
 * 内核模块入口。
 * 
 * @param params 启动参数。
 * 
 * @return 不再返回。
 */
noreturn void aos_kernel_entry(aos_boot_params* params)
{
    init_reset_all_aps(params);
    line("[aos.kernel.entry] 你好.");
    while(true)
    {
        sleep();
    }
}
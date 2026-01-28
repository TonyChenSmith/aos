/**
 * 内核模块入口。
 * @date 2026-01-26
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>

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
 * 读取64位MSR。
 * 
 * @param msr 寄存器地址。
 * 
 * @return 读取的寄存器值。
 */
static inline uint64 init_rdmsr(uint32 msr) {
    uint32 low,high;
    __asm__ volatile("rdmsr":"=a"(low),"=d"(high):"c"(msr));
    return ((uint64)high<<32)|low;
}

/**
 * 写入64位MSR。
 * 
 * @param msr   寄存器地址。
 * @param value 写入寄存器的值。
 * 
 * @return 无返回值。
 */
static inline void init_wrmsr(uint32 msr,uint64 value) {
    __asm__ volatile("wrmsr"::"a"(value&0xFFFFFFFF),"d"(value>>32),"c"(msr));
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
        uint64 apic_base=init_rdmsr(0x1B)&0xFFFFF000;
        uint32* icr_low=(uint32*)(apic_base+0x300);
        uint32* icr_high=(uint32*)(apic_base+0x310);

        *icr_high=0;
        uint32 command=(5<<8)|BIT14|(3<<18);
        *icr_low=command;
        __asm__ volatile("mfence":::"memory");
    }
    else
    {
        init_wrmsr(0x830,(5<<8)|BIT14|(3<<18));
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
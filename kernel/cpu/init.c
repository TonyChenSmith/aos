/**
 * 内核CPU管理模块初始化代码。
 * @date 2026-06-11
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <support/control.h>
#include <support/io.h>

#include "cpui.h"

/**
 * 初始化FPU/SIMD管理。
 * 
 * @return 无返回值。
 */
static void init_fpu_init(void)
{
    x86_fpu_init();
    x86_write_mxcsr(0x1F80);

    uint32 l1ecx,l1edx;
    x86_cpuid(1,null,null,&l1ecx,&l1edx);
    if(l1ecx&BIT26)
    {
        /*支持XSAVE*/
        uint32 ld0eax,ld0edx;
        x86_cpuid_ex(0xD,0,&ld0eax,null,null,&ld0edx);

        uintn cr4=x86_read_cr4();
        cr4|=BIT18;
        x86_write_cr4(cr4);

        /*确认支持特性，默认必须支持x87+SSE*/
        uint64 xcr0=BIT0|BIT1;

        /*确认AVX*/
        if(l1ecx&BIT28&&ld0eax&BIT2)
        {
            xcr0|=BIT2;
        }
        x86_write_xcr0(xcr0);

        uint32 ld0ebx,ld1eax;
        x86_cpuid_ex(0xD,0,null,&ld0ebx,null,null);
        x86_cpuid_ex(0xD,1,&ld1eax,null,null,null);

        if(ld1eax&BIT0)
        {
            cpu_set_fpu_control_mode(FPU_CONTROL_XOSR,64,ld0ebx);
        }
        else
        {
            cpu_set_fpu_control_mode(FPU_CONTROL_XSR,64,ld0ebx);
        }
    }
    else
    {
        cpu_set_fpu_control_mode(FPU_CONTROL_FXSR,16,512);
    }
}

/**
 * 初始化控制寄存器。
 * 按理说引导模块该提前解决问题，先提前防御性编程，等收集所有更新需求后一起编入引导模块。
 * 
 * @return 无返回值。
 */
static void init_control_registers(void)
{
    uintn cr0=x86_read_cr0();
    /*清除CD[30]、NW[29]、TS[3]、EM[2]，不走惰性切换*/
    cr0&=~((uintn)(BIT30|BIT29|BIT3|BIT2));
    /*设置AM[18]、WP[16]、NE[5]、MP[1]*/
    cr0|=BIT18|BIT16|BIT5|BIT1;
    x86_write_cr0(cr0);

    /*CR2不需要设置*/
    /*CR3走页表管理，无需关心*/

    uintn cr4=x86_read_cr4();
    /*仅保留PSE[4]、PAE[5]、MCE[6]、PGE[7]、OSFXSR[9]、OSXMMEXCPT[10]、LA57[12]*/
    cr4&=BIT4|BIT5|BIT7|BIT9|BIT10|BIT12;
    cr4|=BIT9|BIT10;
    /*检查MCE[6]能不能用，能用就设置*/
    uint32 l1edx=0;
    x86_cpuid(1,null,null,null,&l1edx);
    if(l1edx&BIT7)
    {
        cr4|=BIT6;
    }
    x86_write_cr4(cr4);
}

/**
 * 通过启动参数初始化CPU管理模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_cpu_init(aos_boot_params* params)
{
    init_control_registers();
    init_fpu_init();
}
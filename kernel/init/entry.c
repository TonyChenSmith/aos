/**
 * 内核模块入口。
 * @date 2026-01-26
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <support/control.h>

#include "display.h"

/**
 * 内核模块入口。
 * 
 * @param params 启动参数。
 * 
 * @return 不再返回。
 */
noreturn void aos_kernel_entry(aos_boot_params* params)
{
    while(true)
    {
        params->system_table->runtime->reset_system(AOS_EFI_RESET_COLD,0,0,null);
        x86_cpu_halt();
    }
}
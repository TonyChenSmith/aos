/**
 * 内核模块入口。
 * @date 2026-01-26
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <console/output.h>
#include <init/module.h>
#include <support/control.h>


/**
 * 内核模块入口。
 * 
 * @param params 启动参数。
 * 
 * @return 不再返回。
 */
noreturn void aos_kernel_entry(aos_boot_params* params)
{
    if(!kernel_console_output_init(params))
    {
        while(true)
        {
            params->system_table->runtime->reset_system(AOS_EFI_RESET_SHUTDOWN,0,0,null);
            x86_cpu_halt();
        }
    }

    console_graphics_print("\x1b[1;1HAOS ANSI test");

    console_graphics_print("\x1b[3;1H\x1b[31mRED \x1b[32mGREEN \x1b[34mBLUE \x1b[0mDEFAULT");
    console_graphics_print("\x1b[4;1H\x1b[91mBRED \x1b[44mBG-BLUE \x1b[49m \x1b[39mBACK");
    console_graphics_print("\x1b[5;1H\x1b[38;2;255;128;0mTRUECOLOR \x1b[38;5;46mIDX46 \x1b[0mDEFAULT");

    console_graphics_print("\x1b[7;20HCUP(7,20)");

    console_graphics_print("\x1b[9;1HEL0: erase to end ->\x1b[K");
    console_graphics_print("\x1b[10;1HEL2: this line must be blank\x1b[2K");

    console_graphics_print("\x1b[12;1HSTART\x1b[4C4COL\x1b[1AUP");

    console_graphics_print("\x1b[15;1Hbreak by LF: \x1b[31\nm");
    console_graphics_print("\x1b[17;1Hbreak by TAB: \x1b[31\tTAB");
    console_graphics_print("\x1b[18;1Hbreak by CJK: \x1b[31中");
    console_graphics_print("\x1b[19;1Hcross print: \x1b[");
    console_graphics_print("31m literal");

    console_graphics_print("\x1b[21;1Hprivate: \x1b[?25lIGNORED");
    console_graphics_print("\x1b[22;1Hunknown: \x1b[1 qIGNORED");

    console_debugcon_print("rows=%U cols=%U\n",(uint64)get_screen_rows(),(uint64)get_screen_columns());

    /*固定起点*/
    console_graphics_print("\x1b[2J\x1b[H");

    for(uintn index=0;index<get_screen_rows();index++)
    {
        console_graphics_print("FILL %U\r\n",(uint64)index);
    }

    console_graphics_print("SCROLL-1\r\n");
    console_graphics_print("SCROLL-2\r\n");
    console_graphics_print("SCROLL-3\r\n");

    console_graphics_print("\x1b[1;10H<TOP@1");
    console_graphics_print("\x1b[%U;10H<BOTTOM@%U",(uint64)get_screen_rows(),(uint64)get_screen_rows());

    console_graphics_print("\x1b[10;1H\x1b[2K<EL2-OK>");

    console_graphics_print("\x1b[20;1H\x1b[31m<RED@20>");
    console_graphics_print("\x1b[21;1H\x1b[0m<WHITE@21>");

    /*固定起点*/
    console_graphics_print("\x1b[2J\x1b[H");

    /*73 行 = 2*rows-1，正好滚 37 次，top_row 从 0 绕回 0*/
    for(uintn index=0;index<get_screen_rows()*2-1;index++)
    {
        console_graphics_print("FILL %U\r\n",(uint64)index);
    }

    /*到这里 top_row 应该正好是 0，之后只做不滚动的定位*/
    console_graphics_print("\x1b[1;10H<TOP@1>");
    console_graphics_print("\x1b[%U;10H<BOTTOM@%U>",(uint64)get_screen_rows(),(uint64)get_screen_rows());

    while(true)
    {
        x86_cpu_halt();
    }
}
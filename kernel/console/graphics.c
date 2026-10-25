/**
 * 控制台图形显示。
 * 在这里操作的图形操作都认为是帧缓冲已经独占的状态。
 * @date 2026-10-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "init/params.h"

/**
 * 当前帧缓冲。
 */
static void* framebuffer=null;

/**
 * 当前帧缓冲副本。
 */
static void* framebuffer_copy=null;

/**
 * 帧缓冲大小。
 */
static uintn framebuffer_size=0;

/**
 * 当前帧缓冲副本大小。
 */
static uintn framebuffer_copy_size=0;

/**
 * 初始化控制台图形显示。
 * 
 * @param params 启动参数。
 * 
 * @return 成功初始化返回真。
 */
bool console_graphics_init(const aos_boot_params* params)
{
    if(params==null||params->graphics.hres==0||params->graphics.vres==0||params->graphics.framebuffer_base==0||
        params->graphics.scan_line<params->graphics.hres)
    {
        return false;
    }

    uintn test_base=-2*SIZE_512GB;
    uintn test_limit=-SIZE_512GB;

    if(params->graphics.kernel_fb_base<test_base||params->graphics.kernel_fb_base>=test_limit)
    {
        return false;
    }

    test_base=-(3*SIZE_512GB-SIZE_128GB);
    test_limit=-(3*SIZE_512GB-SIZE_256GB);

    if(params->graphics.kernel_copy_base<test_base||params->graphics.kernel_copy_base>=test_limit)
    {
        return false;
    }

    if(params->graphics.framebuffer_size>params->graphics.kernel_copy_size)
    {
        return false;
    }

    return true;
}
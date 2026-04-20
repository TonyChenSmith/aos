/**
 * 内核UEFI运行时固件服务。x86平台的唯一配置。
 * @date 2026-04-19
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <firmware/uefi.h>
#include <init/params.h>
#include <panic/callback.h>

/**
 * UEFI运行时服务。
 */
static aos_efi_runtime_services* runtime=null;

/**
 * 固件恐慌回调。这里主要是手动解锁运行时服务，保证恐慌时能正常使用运行时服务。
 * 
 * @return 无返回值。
 */
static void firmware_panic_callback(void)
{

}

/**
 * 固件恐慌回调结点。
 */
static panic_callback_node firmware_node={null,null,firmware_panic_callback};

/**
 * 通过启动参数初始化固件模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_firmware_init(aos_boot_params *params)
{
    runtime=params->system_table->runtime;
    register_panic_callback(&firmware_node);
}
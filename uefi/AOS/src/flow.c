/* 
 * 模块“aos.uefi”流程控制。
 * 实现了相关的跨文件与跨模块使用函数。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "flow.h"

/* 
 * 模块“aos.uefi”的入口函数。
 * 
 * @param image_handle 模块句柄。
 * @param system_table 系统表。
 * 
 * @return 理论上不返回。
 */
EFI_STATUS EFIAPI aos_uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table)
{
    EFI_STATUS status;

    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Module aos.uefi Debug Information\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ==================================================\n"));

    /*关闭关门狗*/
    status=gBS->SetWatchdogTimer(0,0,0,NULL);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to shut down the watchdog.\n"));
        return status;
    }

    /*初始化内存池*/
    UINTN bitmap,meta;
    status=uefi_pmm_init(&bitmap,&meta);
    if(EFI_ERROR(status))
    {
        return status;
    }

    /*初始化启动参数与运行时环境*/
    aos_boot_params* params=uefi_pool_alloc(sizeof(aos_boot_params));
    ASSERT(params!=NULL);
    params->bitmap_base=bitmap;
    params->tlsf_base=meta;
    status=uefi_env_init(params);
    if(EFI_ERROR(status))
    {
        return status;
    }

    uefi_dump_pool_info();

    CpuDeadLoop();

    return EFI_SUCCESS;
}

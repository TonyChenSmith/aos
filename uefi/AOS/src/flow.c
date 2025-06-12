/*
 * 模块“aos.uefi”流程控制实现。
 * 实现了在UEFI阶段的流程函数，包括模块入口。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
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
 * @return 理论上无返回值，返回值为EFI_SUCCESS。
 */
EFI_STATUS EFIAPI aos_uefi_entry(IN EFI_HANDLE image_handle,IN EFI_SYSTEM_TABLE* system_table)
{
    EFI_STATUS status;

    DEBUG((DEBUG_INFO,"[aos.uefi] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi] Module aos.uefi Debug Information\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi] ==================================================\n"));

    /*关闭关门狗*/
    status=gBS->SetWatchdogTimer(0,0,0,NULL);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi] Error: Failed to shut down the watchdog.\n"));
        return status;
    }

    /*初始化内存池*/
    UINTN bitmap,meta;
    status=uefi_memory_init(&bitmap,&meta);
    if(EFI_ERROR(status))
    {
        return status;
    }

    CpuDeadLoop();

    return EFI_SUCCESS;
}

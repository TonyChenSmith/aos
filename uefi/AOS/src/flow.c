/*
 * 模块“aos.uefi”流程控制。实现了在UEFI阶段的流程函数，包括模块入口。
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

    status=gBS->SetWatchdogTimer(0,0,0,NULL);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi] Error: Failed to shut down the watchdog.\n"));
        return status;
    }

    UINTN bitmap,meta;
    status=uefi_memory_init(&bitmap,&meta);
    if(EFI_ERROR(status))
    {
        return status;
    }

    /*测试部分*/
    DEBUG((DEBUG_INFO,"[aos.uefi] Init state.\n"));
    memory_dump_pool_info();
    DEBUG((DEBUG_INFO,"[aos.uefi] Allocate 3 pools.\n"));
    VOID* a=memory_pool_alloc(23);
    VOID* b=memory_pool_alloc(180);
    VOID* c=memory_pool_alloc(8196);
    DEBUG((DEBUG_INFO,"[aos.uefi] A pool: 0x%016X.\n",(UINTN)a));
    DEBUG((DEBUG_INFO,"[aos.uefi] B pool: 0x%016X.\n",(UINTN)b));
    DEBUG((DEBUG_INFO,"[aos.uefi] C pool: 0x%016X.\n",(UINTN)c));
    DEBUG((DEBUG_INFO,"[aos.uefi] A state.\n"));
    memory_dump_pool_info();
    memory_pool_free(a);
    DEBUG((DEBUG_INFO,"[aos.uefi] B state.\n"));
    memory_dump_pool_info();
    memory_pool_free(c);
    DEBUG((DEBUG_INFO,"[aos.uefi] C state.\n"));
    memory_dump_pool_info();
    memory_pool_free(b);
    DEBUG((DEBUG_INFO,"[aos.uefi] D state.\n"));
    memory_dump_pool_info();

    CpuDeadLoop();

    return EFI_SUCCESS;
}

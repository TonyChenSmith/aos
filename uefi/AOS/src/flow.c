/**
 * 模块运行流程管理。
 * @date 2025-06-01
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "flow.h"

/**
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
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Shut down the watchdog.\n"));
    status=gBS->SetWatchdogTimer(0,0,0,NULL);
    if(EFI_ERROR(status))
    {
        /*关闭看门狗失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to shut down the watchdog.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*初始化引导内存池*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Initializing the boot memory pool.\n"));
    UINTN meta;
    status=mem_init(&meta);
    if(EFI_ERROR(status))
    {
        /*初始化引导内存池失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to initialize the boot memory pool.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*初始化启动参数与启动环境*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Initializing boot parameters and boot environment.\n"));
    aos_boot_params* params=umalloc(sizeof(aos_boot_params));
    ZeroMem(params,sizeof(aos_boot_params));
    ASSERT(params!=NULL);
    params->bpool_base=meta;
    params->bpool_pages=CONFIG_BOOTSTRAP_POOL;
    status=env_init(params);
    if(EFI_ERROR(status))
    {
        /*初始化启动参数与启动环境失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] "
            "Failed to initialize boot parameters and boot environment.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*初始化页表与线性区管理*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Initializing page tables and vma management.\n"));
    status=pvm_init(params);
    if(EFI_ERROR(status))
    {
        /*初始化页表与线性区管理失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] "
            "Failed to initialize page tables and vma management.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*初始化文件系统管理*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Initializing filesystem management.\n"));
    status=fsm_init(params);
    if(EFI_ERROR(status))
    {
        /*初始化文件系统管理失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to initialize filesystem management.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*载入内核文件*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Loading kernel file.\n"));
    status=load_kernel(params);
    if(EFI_ERROR(status))
    {
        /*载入内核文件失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to load kernel file.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }
    
    dump_pool_info((VOID*)params->bpool_base);
    dump_vma();

    CpuDeadLoop();

    return EFI_SUCCESS;
}

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
 * 输出系统表信息。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_system_table()
{
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] UEFI System Table Info\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Address:0x%016lX\n",gST));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Signature:0x%016lX\n",gST->Hdr.Signature));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Revision:0x%08lX\n",gST->Hdr.Revision));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] HeaderSize:0x%08lX\n",gST->Hdr.HeaderSize));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] CRC32:0x%08lX\n",gST->Hdr.CRC32));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] FirmwareVendor:%ls\n",gST->FirmwareVendor));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] FirmwareRevision:0x%08lX\n",gST->FirmwareRevision));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ConsoleInHandle:0x%016lX\n",gST->ConsoleInHandle));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ConIn:0x%016lX\n",gST->ConIn));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ConsoleOutHandle:0x%016lX\n",gST->ConsoleOutHandle));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ConOut:0x%016lX\n",gST->ConOut));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] StandardErrorHandle:0x%016lX\n",gST->StandardErrorHandle));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] StdErr:0x%016lX\n",gST->StdErr));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] RuntimeServices:0x%016lX\n",gST->RuntimeServices));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] BootServices:0x%016lX\n",gST->BootServices));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] NumberOfTableEntries:0x%016lX\n",gST->NumberOfTableEntries));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ConfigurationTable:0x%016lX\n",gST->ConfigurationTable));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ==================================================\n"));
    DEBUG_CODE_END();
}

/**
 * 输出启动参数信息。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI dump_boot_params(IN aos_boot_params* params)
{
    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ==================================================\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] AOS Boot Params Info\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Address:0x%016lX\n",params));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] page_table:0x%016lX\n",params->page_table));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] gbase:0x%016lX\n",params->kinfo.gbase));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] GDT base:0x%016lX\n",params->minfo.fblock_paddr[3]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] ==================================================\n"));
    dump_vma();
    dump_pool_info((VOID*)params->minfo.fblock_paddr[0]);
    dump_bitmap();
    dump_page_table();
    DEBUG_CODE_END();
}

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
    params->minfo.fblock_paddr[0]=meta;
    params->minfo.fblock_pages[0]=CONFIG_BOOTSTRAP_POOL;
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

    /*获取内存映射图*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Get the memory map.\n"));
    UINT32 version;
    UINTN map_key;
    status=get_memory_map(params,&map_key,&version);
    if(EFI_ERROR(status))
    {
        /*获取内存映射图失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to get the memory map.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*关闭启动服务*/
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Exit boot services.\n"));
    status=gBS->ExitBootServices(gImageHandle,map_key);
    if(EFI_ERROR(status))
    {
        /*关闭启动服务失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.flow] Failed to exit boot services.\n"));
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }
    DisableInterrupts();

    /*切换到内核页表*/
    use_kernel_page_table(params);
    status=gRT->SetVirtualAddressMap(params->minfo.map_length,params->minfo.map_entry_size,version,
        (EFI_MEMORY_DESCRIPTOR*)params->minfo.memory_map);
    if(EFI_ERROR(status))
    {
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    /*进入内核*/
    aos_kernel_trampoline trampoline=(aos_kernel_trampoline)params->kinfo.entry;
    trampoline(params,params->kinfo.sbase+EFI_PAGES_TO_SIZE(params->minfo.fblock_pages[4]));

    UNREACHABLE();
    gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
    return EFI_DEVICE_ERROR;
}

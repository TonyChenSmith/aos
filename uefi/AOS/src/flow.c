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
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Size:0x%lX\n",sizeof(aos_boot_params)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]bitmap:0x%016lX\n",OFFSET_OF(aos_boot_params,bitmap)
        ,params->bitmap));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]bitmap_length:0x%016lX\n",
        OFFSET_OF(aos_boot_params,bitmap_length),params->bitmap_length));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]acpi:0x%016lX\n",OFFSET_OF(aos_boot_params,acpi),
        params->acpi));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]smbios:0x%016lX\n",OFFSET_OF(aos_boot_params,smbios),
        params->smbios));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]cpus_length:%lu\n",
        OFFSET_OF(aos_boot_params,cpus_length),params->cpus_length));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]cpus:0x%016lX\n",OFFSET_OF(aos_boot_params,cpus),
        params->cpus));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]graphics_device:0x%016lX\n",
        OFFSET_OF(aos_boot_params,graphics_device),params->graphics_device));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]esp:0x%016lX\n",OFFSET_OF(aos_boot_params,esp),
        params->esp));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]page_table:0x%016lX\n",
        OFFSET_OF(aos_boot_params,page_table),params->esp));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vma_head:0x%016lX\n",
        OFFSET_OF(aos_boot_params,vma_head),params->vma_head));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vma_tail:0x%016lX\n",
        OFFSET_OF(aos_boot_params,vma_tail),params->vma_tail));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]system_table:0x%016lX\n",
        OFFSET_OF(aos_boot_params,system_table),params->system_table));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]features\n",OFFSET_OF(aos_boot_params,features)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Size:0x%lX\n",sizeof(aos_cpu_features)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]magic:0x%02lX\n",OFFSET_OF(aos_cpu_features,magic),
        params->features.magic));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]baseline:0x%02lX\n",
        OFFSET_OF(aos_cpu_features,baseline),params->features.baseline));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vendor:0x%02lX\n",OFFSET_OF(aos_cpu_features,vendor),
        params->features.vendor));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]features:0x%02lX\n",
        OFFSET_OF(aos_cpu_features,features),params->features.features));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]state\n",OFFSET_OF(aos_boot_params,state)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Size:0x%lX\n",sizeof(aos_cpu_state)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]magic:0x%02lX\n",OFFSET_OF(aos_cpu_state,magic),
        params->state.magic));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]state:0x%02lX\n",OFFSET_OF(aos_cpu_state,state),
        params->state.state));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vmtrr:0x%02lX\n",OFFSET_OF(aos_cpu_state,vmtrr),
        params->state.vmtrr));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]apic:0x%02lX\n",OFFSET_OF(aos_cpu_state,apic),
        params->state.apic));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]graphics\n",OFFSET_OF(aos_boot_params,graphics)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Size:0x%lX\n",sizeof(aos_graphics_info)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]hres:%u\n",OFFSET_OF(aos_graphics_info,hres),
        params->graphics.hres));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vres:%u\n",OFFSET_OF(aos_graphics_info,vres),
        params->graphics.vres));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fb_base:0x%016lX\n",
        OFFSET_OF(aos_graphics_info,fb_base),params->graphics.fb_base));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fb_size:0x%016lX\n",
        OFFSET_OF(aos_graphics_info,fb_size),params->graphics.fb_size));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]red:0x%08X\n",OFFSET_OF(aos_graphics_info,red),
        params->graphics.red));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]green:0x%08X\n",OFFSET_OF(aos_graphics_info,green),
        params->graphics.green));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]blue:0x%08X\n",OFFSET_OF(aos_graphics_info,blue),
        params->graphics.blue));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]reserved:0x%08X\n",
        OFFSET_OF(aos_graphics_info,reserved),params->graphics.reserved));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]scan_line:%u\n",
        OFFSET_OF(aos_graphics_info,scan_line),params->graphics.scan_line));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]kinfo\n",OFFSET_OF(aos_boot_params,kinfo)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Size:0x%lX\n",sizeof(aos_kernel_info)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]kbase:0x%016lX\n",OFFSET_OF(aos_kernel_info,kbase),
        params->kinfo.kbase));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]sbase:0x%016lX\n",OFFSET_OF(aos_kernel_info,sbase),
        params->kinfo.sbase));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]gbase:0x%016lX\n",OFFSET_OF(aos_kernel_info,gbase),
        params->kinfo.gbase));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]entry:0x%016lX\n",OFFSET_OF(aos_kernel_info,entry),
        params->kinfo.entry));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]load:%lu\n",OFFSET_OF(aos_kernel_info,load),
        params->kinfo.load));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]start:0x%016lX\n",OFFSET_OF(aos_kernel_info,start),
        params->kinfo.start));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]size:0x%016lX\n",OFFSET_OF(aos_kernel_info,size),
        params->kinfo.size));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]flags:0x%016lX\n",OFFSET_OF(aos_kernel_info,flags),
        params->kinfo.flags));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] --------------------------------------------------\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]minfo\n",OFFSET_OF(aos_boot_params,minfo)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] Size:0x%lX\n",sizeof(aos_memory_info)));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]memory_map:0x%016lX\n",
        OFFSET_OF(aos_memory_info,memory_map),params->minfo.memory_map));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]map_length:0x%016lX\n",
        OFFSET_OF(aos_memory_info,map_length),params->minfo.map_length));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]map_entry_size:0x%016lX\n",
        OFFSET_OF(aos_memory_info,map_entry_size),params->minfo.map_entry_size));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vbase:0x%016lX\n",OFFSET_OF(aos_memory_info,vbase),
        params->minfo.vbase));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_paddr[0]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_paddr[0]),params->minfo.fblock_paddr[0]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_pages[0]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_pages[0]),params->minfo.fblock_pages[0]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_paddr[1]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_paddr[1]),params->minfo.fblock_paddr[1]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_pages[1]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_pages[1]),params->minfo.fblock_pages[1]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_paddr[2]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_paddr[2]),params->minfo.fblock_paddr[2]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_pages[2]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_pages[2]),params->minfo.fblock_pages[2]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_paddr[3]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_paddr[3]),params->minfo.fblock_paddr[3]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_pages[3]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_pages[3]),params->minfo.fblock_pages[3]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_paddr[4]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_paddr[4]),params->minfo.fblock_paddr[4]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]fblock_pages[4]:0x%016lX\n",
        OFFSET_OF(aos_memory_info,fblock_pages[4]),params->minfo.fblock_pages[4]));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vblock_paddr:0x%016lX\n",
        OFFSET_OF(aos_memory_info,vblock_paddr),params->minfo.vblock_paddr));
    DEBUG((DEBUG_INFO,"[aos.uefi.flow] [0x%03lX]vblock_pages:0x%016lX\n",
        OFFSET_OF(aos_memory_info,vblock_pages),params->minfo.vblock_pages));
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
    params->system_table=(aos_efi_system_table*)((UINTN)gST+params->minfo.vbase);

    dump_boot_params(params);

    /*进入内核*/
    aos_kernel_trampoline trampoline=(aos_kernel_trampoline)params->kinfo.entry;
    trampoline(params,params->kinfo.sbase+EFI_PAGES_TO_SIZE(params->minfo.fblock_pages[4]));

    UNREACHABLE();
    gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
    return EFI_DEVICE_ERROR;
}

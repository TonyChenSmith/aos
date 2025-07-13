/* 
 * 模块“aos.uefi”运行环境管理实现。
 * 实现了运行环境管理的相关函数，以及便于调试该部分功能的函数。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "env_internal.h"

/* 
 * 在UEFI阶段设置CPU必要功能。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现必要功能不支持返回错误。
 */
STATIC EFI_STATUS EFIAPI env_set_cpu(IN boot_params* params)
{
    UINT32 eax,ebx,ecx,edx;

    /*CPU类型编号与两个是最大功能号*/
    UINT32 main_max,extended_max;

    /*检测x86_64处理器类型*/
    AsmCpuid(CPUID_SIGNATURE,&eax,&ebx,&ecx,&edx);
    if(ebx==CPUID_SIGNATURE_GENUINE_INTEL_EBX&&ecx==CPUID_SIGNATURE_GENUINE_INTEL_ECX&&
        edx==CPUID_SIGNATURE_GENUINE_INTEL_EDX)
    {
        params->features.vendor=AOS_FEATURE_VENDOR_INTEL;
        main_max=eax;
    }
    else if(ebx==CPUID_SIGNATURE_AUTHENTIC_AMD_EBX&&ecx==CPUID_SIGNATURE_AUTHENTIC_AMD_ECX&&
        edx==CPUID_SIGNATURE_AUTHENTIC_AMD_EDX)
    {
        params->features.vendor=AOS_FEATURE_VENDOR_AMD;
        main_max=eax;
    }
    else
    {
        /*未兼容的x86-64架构处理器，停止运行*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "Cannot identify x86-64 processor model, unable to configure this CPU.\n"));
        return EFI_UNSUPPORTED;
    }

    AsmCpuid(CPUID_EXTENDED_FUNCTION,&eax,&ebx,&ecx,&edx);
    extended_max=eax;

    /*检测我们检测所需最大页数*/
    if(main_max<CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "CPUID leaf 0x%X is not supported. (Max basic leaf: 0x%X)\n",
            CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS,main_max));
        return EFI_UNSUPPORTED;
    }
    if(extended_max<CPUID_EXTENDED_CPU_SIG)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "CPUID leaf 0x%X is not supported. (Max basic leaf: 0x%X)\n",
            CPUID_EXTENDED_CPU_SIG,extended_max));
        return EFI_UNSUPPORTED;
    }
    
    /*必要特性检查*/
    AsmCpuid(CPUID_VERSION_INFO,&eax, &ebx, &ecx, &edx);
    CPUID_VERSION_INFO_EDX ml1edx={.Uint32=edx};
    CPUID_VERSION_INFO_ECX ml1ecx={.Uint32=ecx};
    AsmCpuidEx(CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS,
        CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_SUB_LEAF_INFO,&eax,&ebx,&ecx,&edx);
    CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_ECX ml7ecx={.Uint32=ecx};
    AsmCpuid(CPUID_EXTENDED_CPU_SIG, &eax,&ebx,&ecx,&edx);
    CPUID_EXTENDED_CPU_SIG_ECX el1ecx={.Uint32=ecx};
    CPUID_EXTENDED_CPU_SIG_EDX el1edx={.Uint32=edx};

    if(!ml1edx.Bits.MTRR)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] MTRR is not supported.\n"));
        return EFI_UNSUPPORTED;
    }
    else if(!ml1edx.Bits.PAT)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] PAT is not supported.\n"));
        return EFI_UNSUPPORTED;
    }
    else if(!(ml1edx.Bits.APIC||ml1ecx.Bits.x2APIC))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] APIC is not supported.\n"));
        return EFI_UNSUPPORTED;
    }
    params->features.xapic=ml1edx.Bits.APIC;
    params->features.x2apic=ml1ecx.Bits.x2APIC;

    /*检查APIC启动情况*/
    MSR_IA32_APIC_BASE_REGISTER apic_base={.Uint64=AsmReadMsr64(MSR_IA32_APIC_BASE)};
    if(apic_base.Bits.EN)
    {
        if(apic_base.Bits.EXTD)
        {
            params->state.apic=AOS_STATE_X2APIC;
        }
        else
        {
            params->state.apic=AOS_STATE_XAPIC;
        }
    }
    else
    {
        /*当前没能力打开xAPIC*/
        params->state.apic=AOS_STATE_NO_APIC;
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] xAPIC is not enabled.\n"));
        return EFI_UNSUPPORTED;
    }

    /*基线检查*/
    if(!(ml1edx.Bits.CMOV&&ml1edx.Bits.CX8&&ml1edx.Bits.FPU&&ml1edx.Bits.FXSR&&ml1edx.Bits.MMX&&
        ml1edx.Bits.SSE&&ml1edx.Bits.SSE2&&el1edx.Bits.SYSCALL_SYSRET))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] CPU baseline x86-64-v1 is not supported.\n"));
        return EFI_UNSUPPORTED;
    }
    else
    {
        params->features.baseline=AOS_FEATURE_BASELINE_X86_64_V1;
        if(ml1ecx.Bits.CMPXCHG16B&&el1ecx.Bits.LAHF_SAHF&&ml1ecx.Bits.POPCNT&&ml1ecx.Bits.SSE3&&
            ml1ecx.Bits.SSE4_1&&ml1ecx.Bits.SSE4_2&&ml1ecx.Bits.SSSE3)
        {
            params->features.baseline=AOS_FEATURE_BASELINE_X86_64_V2;
        }
    }
    params->features.page1gb=el1edx.Bits.Page1GB;
    params->features.nx=el1edx.Bits.NX;
    params->features.la57=ml7ecx.Bits.FiveLevelPage;

    /*设置内存属性*/
    BOOLEAN interrupt=SaveAndDisableInterrupts();
    AsmDisableCache();

    IA32_CR4 cr4={.UintN=AsmReadCr4()};
    cr4.Bits.OSFXSR=1;
    cr4.Bits.PGE=0;
    cr4.Bits.SMEP=0;
    cr4.Bits.SMAP=0;
    params->state.la57=cr4.Bits.LA57;
    AsmWriteCr4(cr4.UintN);

    CpuFlushTlb();

    MSR_IA32_MTRRCAP_REGISTER mtrrcap={.Uint64=AsmReadMsr64(MSR_IA32_MTRRCAP)};
    if(mtrrcap.Bits.VCNT==0&&mtrrcap.Bits.FIX==0)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] MTRR is not supported.\n"));
        return EFI_UNSUPPORTED;
    }
    params->state.fmtrr=mtrrcap.Bits.FIX;
    params->state.vmtrr=mtrrcap.Bits.VCNT;

    MSR_IA32_MTRR_DEF_TYPE_REGISTER mtrrdef={.Uint64=AsmReadMsr64(MSR_IA32_MTRR_DEF_TYPE)};
    mtrrdef.Bits.E=0;
    AsmWriteMsr64(MSR_IA32_MTRR_DEF_TYPE,mtrrdef.Uint64);

    if(mtrrcap.Bits.FIX)
    {
        mtrrdef.Bits.FE=1;
        AsmWriteMsr64(MSR_IA32_MTRR_FIX64K_00000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX16K_80000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX16K_A0000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_C0000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_C8000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_D0000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_D8000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_E0000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_E8000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_F0000,ENV_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_F8000,ENV_FIXED_MTRR_TYPE);
    }

    for(UINT8 index=0;index<mtrrcap.Bits.VCNT;index++)
    {
        AsmWriteMsr64(MSR_IA32_MTRR_PHYSBASE0+(index<<1),0);
        AsmWriteMsr64(MSR_IA32_MTRR_PHYSMASK0+(index<<1),0);
    }

    /*PAT设置，替换PAT4与PAT5为WC与WP*/
    MSR_IA32_PAT_REGISTER pat={.Uint64=0};
    pat.Bits.PA0=6;
    pat.Bits.PA1=4;
    pat.Bits.PA2=7;
    pat.Bits.PA3=0;
    pat.Bits.PA4=1;
    pat.Bits.PA5=5;
    pat.Bits.PA6=7;
    pat.Bits.PA7=0;
    params->state.uc=3;
    params->state.wc=4;
    params->state.wt=1;
    params->state.wp=5;
    params->state.wb=0;
    AsmWriteMsr64(MSR_IA32_PAT,pat.Uint64);

    MSR_IA32_EFER_REGISTER efer={.Uint64=AsmReadMsr64(MSR_IA32_EFER)};
    efer.Bits.NXE=el1edx.Bits.NX;
    efer.Bits.SCE=1;
    AsmWriteMsr64(MSR_IA32_EFER,efer.Uint64);

    mtrrdef.Bits.E=1;
    mtrrdef.Bits.Type=MSR_IA32_MTRR_CACHE_WRITE_BACK;
    AsmWriteMsr64(MSR_IA32_MTRR_DEF_TYPE,mtrrdef.Uint64);

    CpuFlushTlb();
    AsmEnableCache();
    cr4.Bits.PGE=1;
    AsmWriteCr4(cr4.UintN);
    SetInterruptState(interrupt);

    /*设置杂项功能*/
    if(params->features.vendor==AOS_FEATURE_VENDOR_INTEL)
    {
        MSR_IA32_MISC_ENABLE_REGISTER misc={.Uint64=AsmReadMsr64(MSR_IA32_MISC_ENABLE)};
        misc.Bits.FastStrings=1;
        AsmWriteMsr64(MSR_IA32_MISC_ENABLE,misc.Uint64);
    }

    return EFI_SUCCESS;
}

/* 
 * 在UEFI阶段设置系统所需表。主要是ACPI、SMBIOS。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
STATIC EFI_STATUS EFIAPI env_set_table(IN boot_params* params)
{
    EFI_STATUS status;
    VOID* table=NULL;
    status=EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid,&table);
    if(EFI_ERROR(status))
    {
        /*不打算兼容老ACPI与无ACPI机子*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] ACPI RSDP is not found.\n"));
        return EFI_UNSUPPORTED;
    }
    else
    {
        ASSERT(table!=NULL);
        params->acpi=(UINTN)table;
    }

    status=EfiGetSystemConfigurationTable(&gEfiSmbios3TableGuid,&table);
    if(EFI_ERROR(status))
    {
        status=EfiGetSystemConfigurationTable(&gEfiSmbiosTableGuid,&table);
        if(EFI_ERROR(status))
        {
            /*不打算兼容无SMBIOS*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] SMBIOS is not found.\n"));
            return EFI_UNSUPPORTED;
        }
        ASSERT(table!=NULL);
        params->smbios=(UINTN)table;
    }
    else
    {
        ASSERT(table!=NULL);
        params->smbios=(UINTN)table;
    }

    UINTN addrs[]={params->acpi,params->smbios};
    EFI_MEMORY_TYPE types[ARRAY_SIZE(addrs)];
    status=memory_get_memory_type(addrs,ARRAY_SIZE(addrs),types);
    if(EFI_ERROR(status))
    {
        return status;
    }
    else
    {
        /*合理假设内存区域应该链式合理*/
        ASSERT(types[0]<EfiMaxMemoryType);
        ASSERT(types[1]<EfiMaxMemoryType);
        if(types[0]==EfiConventionalMemory||types[0]==EfiBootServicesCode||types[0]==EfiBootServicesData||
            types[0]==EfiLoaderCode||types[0]==EfiLoaderData)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] ACPI tables reside in "
                "an unexpected memory type.\n"));
            return EFI_UNSUPPORTED;
        }
        if(types[1]==EfiConventionalMemory||types[1]==EfiBootServicesCode||types[1]==EfiLoaderCode||
            types[1]==EfiLoaderData||types[1]==EfiBootServicesData)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] SMBIOS table resides in "
                "an unexpected memory type.\n"));
            return EFI_UNSUPPORTED;
        }
    }
    return EFI_SUCCESS;
}

/* 
 * 在UEFI阶段计算可用核心数目。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
STATIC EFI_STATUS EFIAPI env_get_cpu_core_info(IN boot_params* params)
{
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp=(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)params->acpi;
    EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* madt=NULL;

    if(rsdp->Revision<EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION)
    {
        /*ACPI结构不正常*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] Unexpected ACPI version.\n"));
        return EFI_UNSUPPORTED;
    }

    ASSERT(EFI_ACPI_1_0_APIC_SIGNATURE==EFI_ACPI_2_0_MULTIPLE_SAPIC_DESCRIPTION_TABLE_SIGNATURE);
    if(rsdp->XsdtAddress)
    {
        EFI_ACPI_DESCRIPTION_HEADER* xsdt=(EFI_ACPI_DESCRIPTION_HEADER*)rsdp->XsdtAddress;
        UINTN count=(xsdt->Length-sizeof(EFI_ACPI_DESCRIPTION_HEADER))>>3;
        UINT64* tables=(UINT64*)((UINTN)xsdt+sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        for(UINTN index=0;index<count;index++)
        {
            if(((EFI_ACPI_DESCRIPTION_HEADER*)tables[index])->Signature==EFI_ACPI_1_0_APIC_SIGNATURE)
            {
                madt=(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)tables[index];
                break;
            }
        }
    }
    else
    {
        EFI_ACPI_DESCRIPTION_HEADER* rsdt=(EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)rsdp->RsdtAddress;
        UINTN count=(rsdt->Length-sizeof(EFI_ACPI_DESCRIPTION_HEADER))>>2;
        UINT32* tables=(UINT32*)((UINTN)rsdt+sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        for(UINTN index=0;index<count;index++)
        {
            if(((EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)tables[index])->Signature==EFI_ACPI_1_0_APIC_SIGNATURE)
            {
                madt=(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)(UINTN)tables[index];
                break;
            }
        }
    }

    if(madt==NULL)
    {
        /*找不到MADT属于意外*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] MADT is not found.\n"));
        return EFI_DEVICE_ERROR;
    }
    else
    {
        env_apic* list=NULL;
        UINT8* bytes=(UINT8*)((UINTN)madt+sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
        ASSERT(madt->Header.Length>sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
        UINTN length=madt->Header.Length-sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);
        UINTN index=0;

        /*两次循环，第一次检查x2APIC，第二次检查xAPIC，尽量避免重复*/
        while(index<length)
        {
            if(bytes[index]==EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC)
            {
                EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE* x2apic=
                    (EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE*)&bytes[index];
                if(x2apic->Flags)
                {
                    env_apic* node;
                    BOOLEAN new_apic=TRUE;
                    if(list!=NULL)
                    {
                        node=list;
                        while(node!=NULL)
                        {
                            if(node->id==x2apic->X2ApicId)
                            {
                                new_apic=FALSE;
                                break;
                            }
                            node=node->next;
                        }
                    }
                    if(new_apic)
                    {
                        node=memory_pool_alloc(sizeof(env_apic));
                        ASSERT(node!=NULL);
                        node->id=x2apic->X2ApicId;
                        node->next=list;
                        list=node;
                    }
                }
            }

            ASSERT(index+1<length);
            index=index+bytes[index+1];
        }

        index=0;
        while(index<length)
        {
            if(bytes[index]==EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC)
            {
                EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE* xapic=
                    (EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE*)&bytes[index];
                if(xapic->Flags)
                {
                    env_apic* node;
                    BOOLEAN new_apic=TRUE;
                    if(list!=NULL)
                    {
                        node=list;
                        while(node!=NULL)
                        {
                            if(node->id==xapic->ApicId)
                            {
                                new_apic=FALSE;
                                break;
                            }
                            node=node->next;
                        }
                    }
                    if(new_apic)
                    {
                        node=memory_pool_alloc(sizeof(env_apic));
                        ASSERT(node!=NULL);
                        node->id=xapic->ApicId;
                        node->next=list;
                        list=node;
                    }
                }
            }

            ASSERT(index+1<length);
            index=index+bytes[index+1];
        }

        if(list==NULL)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] Local APIC is not found.\n"));
            return EFI_DEVICE_ERROR;
        }

        /*重组成数组*/
        UINTN count=0;
        env_apic* node=list;
        while(node!=NULL)
        {
            count++;
            node=node->next;
        }
        UINT32* apics=memory_pool_alloc(count*sizeof(UINT32));
        UINTN itr=0;
        node=list;
        while(node!=NULL)
        {
            apics[itr]=node->id;
            node=node->next;
            memory_pool_free(list);
            list=node;
            itr++;
        }

        for(itr=0;itr<count;itr++)
        {
            for(UINTN j=itr+1;j<count;j++)
            {
                if(apics[itr]>apics[j])
                {
                    UINT32 t=apics[j];
                    apics[j]=apics[itr];
                    apics[itr]=t;
                }
            }
        }

        /*查询一下哪个是BSP，按道理是0*/
        MSR_IA32_APIC_BASE_REGISTER apic_base={.Uint64=AsmReadMsr64(MSR_IA32_APIC_BASE)};
        ASSERT(apic_base.Bits.BSP);

        if(params->state.apic==AOS_STATE_XAPIC)
        {
            UINT32* apic_id=(UINT32*)((apic_base.Uint64&0xFFFFFFFFFFFFF000ULL)+XAPIC_ID_OFFSET);
            UINT32 id=(*apic_id)>>24;
            for(itr=0;itr<count;itr++)
            {
                if(apics[itr]==id)
                {
                    UINT32 t=apics[0];
                    apics[0]=id;
                    apics[itr]=t;
                    break;
                }
            }
        }
        else if(params->state.apic==AOS_STATE_X2APIC)
        {
            UINT32 id=AsmReadMsr32(MSR_IA32_X2APIC_APICID);
            for(itr=0;itr<count;itr++)
            {
                if(apics[itr]==id)
                {
                    UINT32 t=apics[0];
                    apics[0]=id;
                    apics[itr]=t;
                    break;
                }
            }
        }
        else
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] APIC is not enabled.\n"));
            return EFI_DEVICE_ERROR;
        }

        params->cpus_length=count;
        params->cpus=apics;
        return EFI_SUCCESS;
    }
}

/* 
 * 在UEFI阶段，在最低1MB内按页对齐放置GDT。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
STATIC EFI_STATUS EFIAPI env_set_gdt(IN boot_params* params)
{
    params->gdt_size=sizeof(ENV_GDT)+(params->cpus_length<<4);
    EFI_PHYSICAL_ADDRESS base=SIZE_1MB-SIZE_8KB;
    EFI_STATUS status=gBS->AllocatePages(AllocateMaxAddress,EfiRuntimeServicesData,EFI_SIZE_TO_PAGES(params->gdt_size),
        &base);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] System unexpectedly lacks memory for the GDT.\n"));
        return EFI_DEVICE_ERROR;
    }
    UINT64* gdt=(UINT64*)base;
    for(UINTN index=0;index<ARRAY_SIZE(ENV_GDT);index++)
    {
        gdt[index]=ENV_GDT[index];
    }
    params->gdt_base=(UINT32)base;
    return EFI_SUCCESS;
}

/* 
 * 在UEFI阶段初始化运行环境，为引导内核准备。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
EFI_STATUS EFIAPI uefi_env_init(IN boot_params* params)
{
    EFI_STATUS status;
    
    status=env_set_cpu(params);
    if(EFI_ERROR(status))
    {
        return status;
    }

    status=env_set_table(params);
    if(EFI_ERROR(status))
    {
        return status;
    }

    status=env_get_cpu_core_info(params);
    if(EFI_ERROR(status))
    {
        return status;
    }

    status=env_set_gdt(params);
    if(EFI_ERROR(status))
    {
        return status;
    }
    return EFI_SUCCESS;
}
/* 
 * 模块“aos.uefi”运行环境管理。
 * 实现了相关函数。
 * @date 2025-06-11
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "envi.h"

/* 
 * 在UEFI阶段设置CPU必要功能。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现必要功能不支持返回错误。
 */
STATIC EFI_STATUS EFIAPI env_set_cpu(IN OUT aos_boot_params* params)
{
    UINT32 eax,ebx,ecx,edx;
    params->features.magic=AOS_FEATURES_MAGIC;
    params->features.features=0;
    params->state.magic=AOS_STATE_MAGIC;
    params->state.state=0;

    /*CPU类型编号与两个是最大功能号*/
    UINT32 main_max,extended_max;

    /*检测x86_64处理器类型*/
    AsmCpuid(CPUID_SIGNATURE,&eax,&ebx,&ecx,&edx);
    if(ebx==CPUID_SIGNATURE_GENUINE_INTEL_EBX&&ecx==CPUID_SIGNATURE_GENUINE_INTEL_ECX&&
        edx==CPUID_SIGNATURE_GENUINE_INTEL_EDX)
    {
        params->features.vendor=AOS_VENDOR_INTEL;
        main_max=eax;
    }
    else if(ebx==CPUID_SIGNATURE_AUTHENTIC_AMD_EBX&&ecx==CPUID_SIGNATURE_AUTHENTIC_AMD_ECX&&
        edx==CPUID_SIGNATURE_AUTHENTIC_AMD_EDX)
    {
        params->features.vendor=AOS_VENDOR_AMD;
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
            "The CPUID leaf 0x%X is not supported. (Max basic leaf: 0x%X)\n",
            CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS,main_max));
        return EFI_UNSUPPORTED;
    }
    if(extended_max<CPUID_EXTENDED_CPU_SIG)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "The CPUID leaf 0x%X is not supported. (Max basic leaf: 0x%X)\n",
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
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The CPU does not support MTRR.\n"));
        return EFI_UNSUPPORTED;
    }
    else if(!ml1edx.Bits.PAT)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The CPU does not support PAT.\n"));
        return EFI_UNSUPPORTED;
    }
    else if(!(ml1edx.Bits.APIC||ml1ecx.Bits.x2APIC))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The CPU does not support APIC.\n"));
        return EFI_UNSUPPORTED;
    }
    if(ml1edx.Bits.APIC)
    {
        params->features.features|=AOS_FEATURES_XAPIC_BIT;
    }
    if(ml1ecx.Bits.x2APIC)
    {
        params->features.features|=AOS_FEATURES_X2APIC_BIT;
    }

    /*检查APIC启动情况*/
    MSR_IA32_APIC_BASE_REGISTER apic_base={.Uint64=AsmReadMsr64(MSR_IA32_APIC_BASE)};
    if(apic_base.Bits.EN)
    {
        if(apic_base.Bits.EXTD)
        {
            params->state.apic=AOS_APIC_X2APIC;
        }
        else
        {
            params->state.apic=AOS_APIC_XAPIC;
        }
    }
    else
    {
        /*当前没能力打开xAPIC*/
        params->state.apic=AOS_APIC_NO_APIC;
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The xAPIC is not enabled.\n"));
        return EFI_UNSUPPORTED;
    }

    /*基线检查*/
    if(!(ml1edx.Bits.CMOV&&ml1edx.Bits.CX8&&ml1edx.Bits.FPU&&ml1edx.Bits.FXSR&&ml1edx.Bits.MMX&&
        ml1edx.Bits.SSE&&ml1edx.Bits.SSE2&&el1edx.Bits.SYSCALL_SYSRET))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The CPU baseline x86-64-v1 is not supported.\n"));
        return EFI_UNSUPPORTED;
    }
    else
    {
        params->features.baseline=AOS_BASELINE_X86_64_V1;
        if(ml1ecx.Bits.CMPXCHG16B&&el1ecx.Bits.LAHF_SAHF&&ml1ecx.Bits.POPCNT&&ml1ecx.Bits.SSE3&&
            ml1ecx.Bits.SSE4_1&&ml1ecx.Bits.SSE4_2&&ml1ecx.Bits.SSSE3)
        {
            params->features.baseline=AOS_BASELINE_X86_64_V2;
        }
    }
    if(el1edx.Bits.Page1GB)
    {
        params->features.features|=AOS_FEATURES_PAGE1GB_BIT;
    }
    if(el1edx.Bits.NX)
    {
        params->features.features|=AOS_FEATURES_NX_BIT;
    }
    if(ml7ecx.Bits.FiveLevelPage)
    {
        params->features.features|=AOS_FEATURES_LA57_BIT;
    }

    /*设置内存属性*/
    BOOLEAN interrupt=SaveAndDisableInterrupts();
    AsmDisableCache();

    IA32_CR4 cr4={.UintN=AsmReadCr4()};
    cr4.Bits.OSFXSR=1;
    cr4.Bits.PGE=0;
    cr4.Bits.SMEP=0;
    cr4.Bits.SMAP=0;
    if(cr4.Bits.LA57)
    {
        params->state.state|=AOS_STATE_LA57_BIT;
    }
    AsmWriteCr4(cr4.UintN);

    CpuFlushTlb();

    MSR_IA32_MTRRCAP_REGISTER mtrrcap={.Uint64=AsmReadMsr64(MSR_IA32_MTRRCAP)};
    if(mtrrcap.Bits.VCNT==0&&mtrrcap.Bits.FIX==0)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The CPU does not support MTRR.\n"));
        return EFI_UNSUPPORTED;
    }
    if(mtrrcap.Bits.FIX)
    {
        params->state.state|=AOS_STATE_FIXED_MTRR_BIT;
    }
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
    if(params->features.vendor==AOS_VENDOR_INTEL)
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
STATIC EFI_STATUS EFIAPI env_set_table(IN OUT aos_boot_params* params)
{
    EFI_STATUS status;
    VOID* table=NULL;
    status=EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid,&table);
    if(EFI_ERROR(status))
    {
        /*不打算兼容老ACPI与无ACPI机子*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The RSDP is not found.\n"));
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
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] The SMBIOS is not found.\n"));
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
    status=get_memory_type(addrs,ARRAY_SIZE(addrs),types);
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
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] ACPI tables are in unexpected memory.\n"));
            return EFI_UNSUPPORTED;
        }
        if(types[1]==EfiConventionalMemory||types[1]==EfiBootServicesCode||types[1]==EfiLoaderCode||
            types[1]==EfiLoaderData||types[1]==EfiBootServicesData)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] SMBIOS table is in unexpected memory.\n"));
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
STATIC EFI_STATUS EFIAPI env_get_cpu_core_info(IN OUT aos_boot_params* params)
{
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp=(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)params->acpi;
    EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* madt=NULL;

    if(rsdp->Revision<EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION)
    {
        /*ACPI结构不正常*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The ACPI version is unexpected.\n"));
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
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The MADT is not found.\n"));
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
                        node=malloc(sizeof(env_apic));
                        ASSERT(node!=NULL);
                        node->id=x2apic->X2ApicId;
                        node->next=list;
                        list=node;
                    }
                }
            }

            ASSERT(index+1<length);
            index+=bytes[index+1];
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
                        node=malloc(sizeof(env_apic));
                        ASSERT(node!=NULL);
                        node->id=xapic->ApicId;
                        node->next=list;
                        list=node;
                    }
                }
            }

            ASSERT(index+1<length);
            index+=bytes[index+1];
        }

        if(list==NULL)
        {
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] The Local APIC is not found.\n"));
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
        UINT32* apics=malloc(count*sizeof(UINT32));
        UINTN itr=0;
        node=list;
        while(node!=NULL)
        {
            apics[itr]=node->id;
            node=node->next;
            free(list);
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

        if(params->state.apic==AOS_APIC_XAPIC)
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
        else if(params->state.apic==AOS_APIC_X2APIC)
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
            DEBUG((DEBUG_ERROR,"[aos.uefi.env] The APIC is not enabled.\n"));
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
STATIC EFI_STATUS EFIAPI env_set_gdt(IN OUT aos_boot_params* params)
{
    params->gdt_size=sizeof(ENV_GDT)+(params->cpus_length<<4);
    EFI_PHYSICAL_ADDRESS base=SIZE_1MB-SIZE_8KB;
    EFI_STATUS status=gBS->AllocatePages(AllocateMaxAddress,EfiRuntimeServicesData,EFI_SIZE_TO_PAGES(params->gdt_size),
        &base);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "The system unexpectedly lacks sufficient memory for the GDT.\n"));
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
 * 将掩码转为位移与位宽。
 * 
 * @param mask  掩码。
 * @param shift 位移。
 * @param width 位宽。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI env_mask_to_shift_width(IN UINT32 mask,OUT UINT8* shift,OUT UINT8* width)
{
    if(mask==0)
    {
        *shift=0;
        *width=0;
    }
    else
    {
        *shift=(UINT8)LowBitSet32(mask);
        mask>>=*shift;
        *width=0;
        while(mask&BIT0)
        {
            (*width)++;
            mask>>=1;
        }
    }
}

/* 
 * 获取单个像素所占位宽。
 * 
 * @param info 图形输出模式信息。
 * 
 * @return 位宽大小，不支持则返回最大值。
 */
STATIC UINT8 EFIAPI env_get_pixel_bit_width(IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info)
{
    EFI_PIXEL_BITMASK* mask;
    mask=&info->PixelInformation;
    UINT8 shift,width=0;
    switch(info->PixelFormat)
    {
        case PixelRedGreenBlueReserved8BitPerColor:
        case PixelBlueGreenRedReserved8BitPerColor:
            return 32;
        case PixelBitMask:
            env_mask_to_shift_width(mask->BlueMask|mask->GreenMask|mask->RedMask|mask->ReservedMask,
                &shift,&width);
            return width;
        case PixelBltOnly:
        default:
            /*意外情况*/
            return MAX_UINT8;
    }
}

/* 
 * 获取单个像素所占位宽。
 * 
 * @param info 图形输出模式信息。
 * 
 * @return 如果EDID正常返回成功，否则返回不支持。
 */
STATIC EFI_STATUS EFIAPI env_get_edid_info(IN EFI_HANDLE gop,OUT UINT32* hres,OUT UINT32* vres)
{
    EFI_STATUS status;
    /*协议分两种，第一活动，第二发现，不考虑其他渠道*/
    EFI_EDID_ACTIVE_PROTOCOL* edid_active;
    EFI_EDID_DISCOVERED_PROTOCOL* edid_discovered;
    env_edid* edid;

    status=gBS->OpenProtocol(gop,&gEfiEdidActiveProtocolGuid,(VOID**)&edid_active,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(status)||edid_active->SizeOfEdid==0)
    {
        /*不支持或者EDID大小为0*/
        status=gBS->OpenProtocol(gop,&gEfiEdidDiscoveredProtocolGuid,(VOID**)&edid_discovered,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if(EFI_ERROR(status)||edid_discovered->SizeOfEdid<sizeof(env_edid))
        {
            return EFI_UNSUPPORTED;
        }
        else
        {
            edid=(env_edid*)edid_discovered->Edid;
            ASSERT(edid_discovered->SizeOfEdid>=sizeof(env_edid));
        }
    }
    else
    {
        edid=(env_edid*)edid_active->Edid;
        ASSERT(edid_active->SizeOfEdid>=sizeof(env_edid));
    }

    UINT8 check=0;
    UINT8* edid_value=(UINT8*)edid;
    UINT8 index;
    for(index=0;index<128;index++)
    {
        check+=edid_value[index];
    }
    if(check!=0||edid->version_number!=1||edid->detailed_timings[0].pixel_clock==0)
    {
        /*校验失败，版本不支持或者未定义最佳显示，版本号2开始的4k、8k屏幕暂未有EDID支持想法*/
        return EFI_UNSUPPORTED;
    }
    
    *hres=edid->detailed_timings[0].horizontal_active_low|
        (((UINT32)(edid->detailed_timings[0].horizontal_high&0xF0))<<4);
    *vres=edid->detailed_timings[0].vertical_active_low|
        (((UINT32)(edid->detailed_timings[0].vertical_active_low&0xF0))<<4);
    return EFI_SUCCESS;
}

/* 
 * 对标准色彩值按像素格式要求缩放。
 * 
 * @param value 单色色值。
 * @param width 目标位宽。
 * 
 * @return 缩放后的色彩值。
 */
STATIC UINT32 EFIAPI env_scale_color(IN UINT8 value,IN UINT8 width)
{
    if(width==8)
    {
        return value;
    }
    else
    {
        UINT64 max=(1ULL<<width)-1;
        return (UINT32)(((value*max+127)/255)&max);
    }
}

/* 
 * 在指定区域内填充颜色。调用者有责任保证参数合理。
 * 
 * @param pixel     像素颜色。
 * @param size      像素大小。
 * @param scan_line 扫面线像素数。
 * @param fb_base   帧缓冲基址。
 * @param x         起点横坐标。
 * @param y         起点纵坐标。
 * @param w         区域宽度。
 * @param h         区域高度。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI env_fill_color(IN env_graphics_pixel pixel,IN UINT8 size,IN UINT32 scan_line,IN UINTN fb_base,
    IN UINT32 x,IN UINT32 y,IN UINT32 w,IN UINT32 h)
{
    if(size==4)
    {
        UINT32* line=(UINT32*)(fb_base+((y*scan_line+x)<<2));
        for(UINTN j=0;j<h;j++)
        {
            SetMem32(line,w<<2,pixel.pixel);
            line=(UINT32*)((UINTN)line+(scan_line<<2));
        }
    }
    else if(size<4&&size>0)
    {
        UINT8* line=(UINT8*)(fb_base+((y*scan_line+x)*size));
        for(UINTN j=0;j<h;j++)
        {
            for(UINTN i=0;i<w;i++)
            {
                UINTN base=i*size;
                for(UINT8 k=0;k<size;k++)
                {
                    line[base+k]=pixel.data[k];
                }
            }
            line=(UINT8*)((UINTN)line+scan_line*size);
        }
    }
}

/* 
 * 获取图形信息。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
STATIC EFI_STATUS EFIAPI env_get_graphics_info(IN OUT aos_boot_params* params)
{
    EFI_STATUS status;
    EFI_HANDLE* gops=NULL;
    UINTN count=0;

    /* 
     * 寻找显示当前屏幕的图形输出协议。仅支持GOP，EFI 1.X时代的UGA不考虑。
     * 寻找逻辑为搜索所有拥有GOP的句柄，优先考虑带ConsoleOutDevice协议，其次考虑第一个GOP。
     * 上述GOP中不存在仅Blt输出的，这种GOP视为无效。如果没找到GOP，说明不能图形显示，不属于设计目标，报错终止。
     */
    status=gBS->LocateHandle(ByProtocol,&gEfiGraphicsOutputProtocolGuid,NULL,&count,gops);
    if(status==EFI_BUFFER_TOO_SMALL)
    {
        ASSERT(count>0);
        gops=(EFI_HANDLE*)malloc(count);
        status=gBS->LocateHandle(ByProtocol,&gEfiGraphicsOutputProtocolGuid,NULL,&count,gops);
        if(EFI_ERROR(status))
        {
            return status;
        }
        count/=sizeof(EFI_HANDLE);
    }
    else
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The Graphics Output Protocol is not found.\n"));
        return EFI_NOT_FOUND;
    }

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop=NULL;
    EFI_HANDLE target;
    for(UINTN index=0;index<count;index++)
    {
        EFI_GRAPHICS_OUTPUT_PROTOCOL* current;
        status=gBS->OpenProtocol(gops[index],&gEfiGraphicsOutputProtocolGuid,(VOID**)&current,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        ASSERT(status==EFI_SUCCESS);
        ASSERT(current!=NULL);
        if(current->Mode->Info->PixelFormat==PixelBltOnly||current->Mode->Info->PixelFormat>=PixelFormatMax)
        {
            /*非目标GOP*/
            continue;
        }
        status=gBS->OpenProtocol(gops[index],&gEfiConsoleOutDeviceGuid,NULL,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
        if(status==EFI_SUCCESS)
        {
            gop=current;
            target=gops[index];
            break;
        }
        if(gop==NULL)
        {
            gop=current;
            target=gops[index];
        }
    }
    free(gops);
    if(gop==NULL)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] The supported Graphics Output Protocol is not found.\n"));
        return EFI_UNSUPPORTED;
    }
    EFI_DEVICE_PATH_PROTOCOL* device;
    status=gBS->OpenProtocol(target,&gEfiDevicePathProtocolGuid,(VOID**)&device,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "The Device Path Protocol is not found within the handle corresponding to the Graphics Output Protocol\n"));
        return EFI_UNSUPPORTED;
    }
    else
    {
        UINTN size=GetDevicePathSize(device);
        ASSERT(size>0);
        VOID* buffer=malloc(size);
        ASSERT(buffer!=NULL);
        CopyMem(buffer,device,size);
        params->graphics_device=(aos_efi_device_path*)buffer;
    }

    /* 
     * 寻找适合屏幕的最大最佳模式，这一阶段我们相信模式是安全的。查找方式按如下操作：
     * 首先查找EDID信息，如果能找到EDID信息直接按信息搜索模式设置。
     * 其次检查当前显示分辨率，如果当前大小非VGA则保持。（VBox对策）
     * 接着检测0号模式，如果大小为非VGA则设置。（QEMU对策）
     * 最后找模式列表中最大最佳模式。（物理硬件对策）
     */
    UINT32 hlim=MAX_UINT32,vlim=MAX_UINT32;
    status=env_get_edid_info(target,&hlim,&vlim);
    if(status!=EFI_SUCCESS)
    {
        if(gop->Mode->Info->HorizontalResolution>ENV_SVGA_HORIZONTAL_RESOLUTION&&
            gop->Mode->Info->VerticalResolution>ENV_SVGA_VERTICAL_RESOLUTION)
        {
            hlim=gop->Mode->Info->HorizontalResolution;
            vlim=gop->Mode->Info->VerticalResolution;
        }
        else
        {
            UINTN size;
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
            status=gop->QueryMode(gop,0,&size,&info);
            if(EFI_ERROR(status))
            {
                DEBUG((DEBUG_WARN,"[aos.uefi.env] "
                    "A device error occurred while querying the graphics mode. (Mode 0)\n"));
            }
            else
            {
                if(!((info->HorizontalResolution==ENV_VGA_HORIZONTAL_RESOLUTION&&
                    info->VerticalResolution==ENV_VGA_VERTICAL_RESOLUTION)||
                    (info->HorizontalResolution==ENV_SVGA_HORIZONTAL_RESOLUTION&&
                    info->VerticalResolution==ENV_SVGA_VERTICAL_RESOLUTION)))
                {
                    hlim=info->HorizontalResolution;
                    vlim=info->VerticalResolution;
                }
                gBS->FreePool(info);
            }
        }
    }
    ASSERT(hlim>0&&vlim>0);
    UINT32 max=gop->Mode->MaxMode;
    UINT32 mode=max;
    UINT32 hres;
    UINT32 vres;
    UINT8 depth;
    UINTN size;
    for(UINT32 index=0;index<max;index++)
    {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
        status=gop->QueryMode(gop,index,&size,&info);
        if(EFI_ERROR(status))
        {
            DEBUG((DEBUG_WARN,"[aos.uefi.env] "
                "A device error occurred while querying the graphics mode. (Mode %u)\n",index));
            continue;
        }
        if(info->HorizontalResolution>hlim||info->VerticalResolution>vlim||info->PixelFormat==PixelBltOnly)
        {
            gBS->FreePool(info);
            continue;
        }
        UINT8 d=env_get_pixel_bit_width(info);
        ASSERT(d<MAX_UINT8);
        if(mode==max)
        {
            mode=index;
            hres=info->HorizontalResolution;
            vres=info->VerticalResolution;
            depth=d;
        }
        else
        {
            if(hres<info->HorizontalResolution)
            {
                mode=index;
                hres=info->HorizontalResolution;
                vres=info->VerticalResolution;
                depth=d;
            }
            else if(hres==info->HorizontalResolution&&vres<info->VerticalResolution)
            {
                mode=index;
                vres=info->VerticalResolution;
                depth=d;
            }
            else if(hres==info->HorizontalResolution&&vres==info->VerticalResolution&&depth<d)
            {
                mode=index;
                depth=d;
            }
        }
        gBS->FreePool(info);
    }
    if(mode==max)
    {
        mode=gop->Mode->Mode;
        DEBUG((DEBUG_WARN,
            "[aos.uefi.env] No valid mode found, using current mode. (Mode %u)\n",mode));
    }

    /* 
     * 设置模式与输出信息。
     */
    if(mode!=gop->Mode->Mode)
    {
        status=gop->SetMode(gop,mode);
        if(EFI_ERROR(status))
        {
            DEBUG((DEBUG_WARN,"[aos.uefi.env] Failed to set graphics mode. (Mode %u)\n",mode));
        }
    }
    params->graphics.hres=gop->Mode->Info->HorizontalResolution;
    params->graphics.vres=gop->Mode->Info->VerticalResolution;
    params->graphics.scan_line=gop->Mode->Info->PixelsPerScanLine;
    params->graphics.fb_base=gop->Mode->FrameBufferBase;
    params->graphics.fb_size=gop->Mode->FrameBufferSize;
    ASSERT(gop->Mode->Info->PixelFormat==PixelBitMask||
        gop->Mode->Info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor||
        gop->Mode->Info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor);
    if(gop->Mode->Info->PixelFormat==PixelBitMask)
    {
        params->graphics.red=gop->Mode->Info->PixelInformation.RedMask;
        params->graphics.green=gop->Mode->Info->PixelInformation.GreenMask;
        params->graphics.blue=gop->Mode->Info->PixelInformation.BlueMask;
        params->graphics.reserved=gop->Mode->Info->PixelInformation.ReservedMask;
    }
    else if(gop->Mode->Info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor)
    {
        params->graphics.red=0xFF0000;
        params->graphics.green=0xFF00;
        params->graphics.blue=0xFF;
        params->graphics.reserved=0xFF000000;
    }
    else
    {
        params->graphics.red=0xFF;
        params->graphics.green=0xFF00;
        params->graphics.blue=0xFF0000;
        params->graphics.reserved=0xFF000000;
    }

    /* 
     * 绘制初始背景色。
     */
    if(CONFIG_FORCE_FILL_GRAPHICS_BACKGROUND)
    {
        UINT8 red_shift,red_width,green_shift,green_width,blue_shift,blue_width,color_size;
        env_mask_to_shift_width(params->graphics.red|params->graphics.green|params->graphics.blue|
            params->graphics.reserved,&red_shift,&red_width);
        ASSERT(red_width>0&&red_width<=32&&red_width%8==0);
        color_size=red_width>>3;
        env_mask_to_shift_width(params->graphics.red,&red_shift,&red_width);
        env_mask_to_shift_width(params->graphics.green,&green_shift,&green_width);
        env_mask_to_shift_width(params->graphics.blue,&blue_shift,&blue_width);
        env_graphics_pixel pixel={.pixel=0};
        pixel.pixel|=(env_scale_color(ENV_BACKGROUND_RED,red_width)<<red_shift);
        pixel.pixel|=(env_scale_color(ENV_BACKGROUND_GREEN,green_width)<<green_shift);
        pixel.pixel|=(env_scale_color(ENV_BACKGROUND_BLUE,blue_width)<<blue_shift);
        env_fill_color(pixel,color_size,params->graphics.scan_line,params->graphics.fb_base,
            0,0,params->graphics.hres,params->graphics.vres);
    }
    return EFI_SUCCESS;
}

/* 
 * 初始化运行环境。
 * 
 * @param parmas 启动参数。
 * 
 * @return 一般成功，出现问题返回错误。
 */
EFI_STATUS EFIAPI env_init(IN OUT aos_boot_params* params)
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

    status=env_get_graphics_info(params);
    if(EFI_ERROR(status))
    {
        return status;
    }
    return EFI_SUCCESS;
}
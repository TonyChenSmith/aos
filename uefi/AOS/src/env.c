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
 * @return 一般成功，出现必要功能不支持返回EFI_UNSUPPORTED。
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
 * @return 一般成功，出现必要功能不支持返回EFI_UNSUPPORTED。
 */
STATIC EFI_STATUS EFIAPI env_get_set_table(IN boot_params* params)
{
    return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI env_get_cpu_core_count(IN boot_params* params)
{
    return EFI_SUCCESS;
}

/* 
 * 在UEFI阶段初始化运行环境，为引导内核准备。
 * 
 * @param parmas 启动参数。
 * 
 * @return 固定返回EFI_SUCCESS。
 */
EFI_STATUS EFIAPI uefi_env_init(IN boot_params* params)
{
    EFI_STATUS status;
    
    status=env_set_cpu(params);
    if(EFI_ERROR(status))
    {
        return status;
    }

    return EFI_SUCCESS;
}
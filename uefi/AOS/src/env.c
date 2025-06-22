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
 * 环境使用的MTRR设置。
 */
STATIC MTRR_SETTINGS env_mtrr_settings={
    .Fixed={
        .Mtrr={[0 ... (MTRR_NUMBER_OF_FIXED_MTRR-1)]=ENV_FIXED_MTRR_TYPE}
    },
    .Variables={
        .Mtrr={[0 ... (MTRR_NUMBER_OF_VARIABLE_MTRR-1)]={
            .Base=0,
            .Mask=0
        }}
    }
};

/*
 * 在UEFI阶段设置CPU功能。
 * 
 * @param parmas 启动参数。
 * 
 * @return 无返回值。
 */
STATIC EFI_STATUS EFIAPI env_set_cpu_features(IN boot_params* params)
{
    UINT32 eax,ebx,ecx,edx;

    AsmCpuid(CPUID_SIGNATURE,&eax, &ebx, &ecx, &edx);
    if(ebx==CPUID_SIGNATURE_GENUINE_INTEL_EBX&&ecx==CPUID_SIGNATURE_GENUINE_INTEL_ECX&&
        edx==CPUID_SIGNATURE_GENUINE_INTEL_EDX)
    {
        /*Intel处理器*/
        
    }
    else if(ebx==CPUID_SIGNATURE_GENUINE_INTEL_EBX&&ecx==CPUID_SIGNATURE_GENUINE_INTEL_ECX&&
        edx==CPUID_SIGNATURE_GENUINE_INTEL_EDX)
    {
        /*AMD处理器*/
    }
    else
    {
        /*未兼容的x86-64架构处理器，停止运行*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.env] "
            "Failed to set the CPU features as the x86_64 processor model could not be identified.\n"));
        return EFI_DEVICE_ERROR;
    }
}

/*
 * 在UEFI阶段设置内存属性。
 * 
 * @param parmas 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI env_set_memory_attribute(IN boot_params* params)
{
    env_mtrr_settings.MtrrDefType=ENV_MTRR_DEF_TYPE.Uint64;

    MSR_IA32_MTRRCAP_REGISTER mttrcap={
        .Uint64=AsmReadMsr64(MSR_IA32_MTRRCAP)
    };
    params->flags.wc=mttrcap.Bits.WC;

    MtrrSetAllMtrrs(&env_mtrr_settings);
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
    env_set_memory_attribute(params);

    return EFI_SUCCESS;
}
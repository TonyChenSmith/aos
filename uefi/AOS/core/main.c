/**
 * 模块主函数。
 * @date 2026-06-27
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Uefi.h>
#include <Guid/Acpi.h>
#include <Guid/SmBios.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Register/Amd/ArchitecturalMsr.h>
#include <Register/Amd/Cpuid.h>
#include <Register/Intel/ArchitecturalMsr.h>
#include <Register/Intel/Cpuid.h>
#include <Register/Intel/LocalApic.h>

#include "config.h"
#include "panic.h"
#include "params.h"
#include "pool.h"

/*配置检查*/

#ifndef CONFIG_BOOTSTRAP_POOL
#error The macro CONFIG_BOOTSTRAP_POOL is undefined.
#endif /*CONFIG_BOOTSTRAP_POOL*/

#ifndef CONFIG_KERNEL_POOL
#error The macro CONFIG_KERNEL_POOL is undefined.
#endif /*CONFIG_KERNEL_POOL*/

#ifndef CONFIG_PAGE_TABLE_POOL
#error The macro CONFIG_PAGE_TABLE_POOL is undefined.
#endif /*CONFIG_PAGE_TABLE_POOL*/

#ifndef CONFIG_KERNEL_STACK
#error The macro CONFIG_KERNEL_STACK is undefined.
#endif /*CONFIG_KERNEL_STACK*/

#ifndef CONFIG_RANDOMIZE_BASE
#error The macro CONFIG_RANDOMIZE_BASE is undefined.
#endif /*CONFIG_RANDOMIZE_BASE*/

#ifndef CONFIG_MINIMUM_FREE_PAGES
#error The macro CONFIG_MINIMUM_FREE_PAGES is undefined.
#endif /*CONFIG_MINIMUM_FREE_PAGES*/

STATIC_ASSERT(CONFIG_BOOTSTRAP_POOL>16,"The macro CONFIG_BOOTSTRAP_POOL must be greater than 16.");
STATIC_ASSERT(CONFIG_BOOTSTRAP_POOL<=128,"The macro CONFIG_BOOTSTRAP_POOL must be less than or equal to 128.");
STATIC_ASSERT(CONFIG_KERNEL_POOL>=CONFIG_BOOTSTRAP_POOL,"The macro CONFIG_KERNEL_POOL must be greater than or equal to"
    " the macro CONFIG_BOOTSTRAP_POOL.");
STATIC_ASSERT(CONFIG_PAGE_TABLE_POOL>0,"The macro CONFIG_PAGE_TABLE_POOL must be greater than 0.");
STATIC_ASSERT(CONFIG_PAGE_TABLE_POOL%64==0,"The macro CONFIG_PAGE_TABLE_POOL must be a multiple of 64.");
STATIC_ASSERT(CONFIG_KERNEL_STACK>8,"The macro CONFIG_KERNEL_STACK must be greater than 8.");
STATIC_ASSERT(CONFIG_KERNEL_STACK%8==0,"The macro CONFIG_KERNEL_STACK must be a multiple of 8.");
STATIC_ASSERT(CONFIG_MINIMUM_FREE_PAGES>=(SIZE_512MB/EFI_PAGE_SIZE),"The macro CONFIG_MINIMUM_FREE_PAGES must be"
    " greater than or equal to 131072.");

/**
 * 在UEFI阶段检查CPU必要功能。
 * 
 * @param parmas 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI main_cpu_check(IN aos_boot_params* params)
{
    UINT32 eax,ebx,ecx,edx;
    params->features.magic=AOS_FEATURES_MAGIC;
    params->features.features=0;
    params->state.magic=AOS_STATE_MAGIC;
    params->state.state=0;

    /*检测x86-64处理器类型*/
    UINT32 main_max,extended_max;
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
        panic(EFI_UNSUPPORTED,"Failed to identify x86-64 processor vendor.");
    }

    /*检测我们检测所需最大页数是否可以访问*/
    AsmCpuid(CPUID_EXTENDED_FUNCTION,&eax,&ebx,&ecx,&edx);
    extended_max=eax;
    if(main_max<CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS)
    {
        panic(EFI_UNSUPPORTED,"CPU does not support CPUID leaf 0x07.");
    }
    if(extended_max<CPUID_EXTENDED_CPU_SIG)
    {
        panic(EFI_UNSUPPORTED,"CPU does not support CPUID leaf 0x80000001.");
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
        panic(EFI_UNSUPPORTED,"CPU does not support MTRR.");
    }
    else if(!ml1edx.Bits.PAT)
    {
        panic(EFI_UNSUPPORTED,"CPU does not support PAT.");
    }
    else if(!(ml1edx.Bits.APIC||ml1ecx.Bits.x2APIC))
    {
        panic(EFI_UNSUPPORTED,"CPU does not support xAPIC or x2APIC.");
    }
    else if(!ml1edx.Bits.TSC)
    {
        panic(EFI_UNSUPPORTED,"CPU does not support TSC.");
    }

    /*检查APIC状态*/
    if(ml1edx.Bits.APIC)
    {
        params->features.features|=AOS_FEATURES_XAPIC;
    }
    if(ml1ecx.Bits.x2APIC)
    {
        params->features.features|=AOS_FEATURES_X2APIC;
    }
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
        params->state.apic=AOS_APIC_NO_APIC;
    }

    /*基线检查和可选特性设置*/
    if(!(ml1edx.Bits.CMOV&&ml1edx.Bits.CX8&&ml1edx.Bits.FPU&&ml1edx.Bits.FXSR&&ml1edx.Bits.MMX&&
        ml1edx.Bits.SSE&&ml1edx.Bits.SSE2&&el1edx.Bits.SYSCALL_SYSRET))
    {
        panic(EFI_UNSUPPORTED,"CPU does not support x86-64-v1 baseline.");
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
        params->features.features|=AOS_FEATURES_PAGE1GB;
    }
    if(el1edx.Bits.NX)
    {
        params->features.features|=AOS_FEATURES_NX;
    }
    if(ml7ecx.Bits.FiveLevelPage)
    {
        params->features.features|=AOS_FEATURES_LA57;
    }
    if(ml1edx.Bits.MCE)
    {
        params->features.features|=AOS_FEATURES_MC;
    }

    /*MTRR属性检查*/
    MSR_IA32_MTRRCAP_REGISTER mtrrcap={.Uint64=AsmReadMsr64(MSR_IA32_MTRRCAP)};
    if(mtrrcap.Bits.VCNT==0&&mtrrcap.Bits.FIX==0)
    {
        panic(EFI_UNSUPPORTED,"CPU does not support MTRR.");
    }
    if(mtrrcap.Bits.FIX)
    {
        params->state.state|=AOS_STATE_FIXED_MTRR;
    }
    params->state.vmtrr=mtrrcap.Bits.VCNT;
}

/**
 * 设置启动参数所需数据表。
 * 
 * @param parmas 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI main_set_table(IN aos_boot_params* params)
{
    UINTN range[3][2];
    VOID* table;
    panic_if_error(EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid,&table),
        "ACPI RSDP not found in UEFI Configuration Table. This system does not support ACPI, which is"
        " required.");
    params->acpi=(UINTN)table;

    EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp=(EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)params->acpi;
    if(rsdp->Revision<EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION)
    {
        panic(EFI_UNSUPPORTED,"This system's ACPI revision is below 5.0, which is required to boot.");
    }
    range[0][0]=params->acpi;
    range[0][1]=rsdp->Length+range[0][0];

    table=NULL;
    BOOLEAN is3=TRUE;
    EFI_STATUS status=EfiGetSystemConfigurationTable(&gEfiSmbios3TableGuid,&table);
    if(EFI_ERROR(status))
    {
        panic_if_error(EfiGetSystemConfigurationTable(&gEfiSmbiosTableGuid,&table),
            "SMBIOS not found in UEFI Configuration Table. This system does not expose SMBIOS,"
            " which is required.");
        is3=FALSE;
        params->smbios=(UINTN)table;
        range[1][0]=params->smbios;
        range[1][1]=range[1][0]+sizeof(SMBIOS_TABLE_ENTRY_POINT);
        SMBIOS_TABLE_ENTRY_POINT* entry=(SMBIOS_TABLE_ENTRY_POINT*)params->smbios;
        range[2][0]=entry->TableAddress;

        UINT8 checksum=0,index=0;
        UINT8* edata=(UINT8*)params->smbios;
        if(CompareMem(entry->AnchorString,SMBIOS_ANCHOR_STRING,
            SMBIOS_ANCHOR_STRING_LENGTH)!=0)
        {
            panic(EFI_DEVICE_ERROR,"SMBIOS entry point anchor mismatch.");
        }
        if(entry->EntryPointLength!=sizeof(SMBIOS_TABLE_ENTRY_POINT))
        {
            panic(EFI_DEVICE_ERROR,"SMBIOS entry point length mismatch.");
        }
        while(index<entry->EntryPointLength)
        {
            checksum+=edata[index++];
        }
        if(checksum!=0)
        {
            panic(EFI_DEVICE_ERROR,"SMBIOS entry point checksum verification failed.");
        }

        UINT8* sm=(UINT8*)range[2][0];
        UINTN tail=(UINTN)sm+entry->TableLength;
        while(TRUE)
        {
            if((UINTN)sm+sizeof(SMBIOS_STRUCTURE)>tail)
            {
                panic(EFI_DEVICE_ERROR,"SMBIOS end-of-table (Type 127) not found within declared table"
                    " size.");
            }
            SMBIOS_STRUCTURE* stru=(SMBIOS_STRUCTURE*)sm;
            if(stru->Type!=SMBIOS_TYPE_END_OF_TABLE)
            {
                if(stru->Length<sizeof(SMBIOS_STRUCTURE))
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X has invalid length %u.",
                        (UINTN)sm-range[2][0],stru->Length);
                }
                if((UINTN)sm+stru->Length>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X formatted area exceeds table"
                        " size.",(UINTN)sm-range[2][0]);
                }
                sm=(UINT8*)((UINTN)sm+stru->Length);
                while(sm[0]!=0||sm[1]!=0)
                {
                    if((UINTN)sm+1>=tail)
                    {
                        panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X has unterminated"
                            " string-list.",(UINTN)stru-range[2][0]);
                    }
                    sm++;
                }
                sm=(UINT8*)((UINTN)sm+2);
            }
            else
            {
                range[2][1]=(UINTN)sm+stru->Length+2;
                break;
            }
        }
    }
    else
    {
        params->smbios=(UINTN)table;
        range[1][0]=params->smbios;
        range[1][1]=range[1][0]+sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT);
        SMBIOS_TABLE_3_0_ENTRY_POINT* entry=(SMBIOS_TABLE_3_0_ENTRY_POINT*)params->smbios;
        range[2][0]=entry->TableAddress;

        UINT8 checksum=0,index=0;
        UINT8* edata=(UINT8*)params->smbios;
        if(CompareMem(entry->AnchorString,SMBIOS_3_0_ANCHOR_STRING,
            SMBIOS_3_0_ANCHOR_STRING_LENGTH)!=0)
        {
            panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 entry point anchor mismatch.");
        }
        if(entry->EntryPointLength!=sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT))
        {
            panic(EFI_DEVICE_ERROR,"SMBIOS entry point length mismatch.");
        }
        while(index<entry->EntryPointLength)
        {
            checksum+=edata[index++];
        }
        if(checksum!=0)
        {
            panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 entry point checksum verification failed.");
        }

        UINT8* sm=(UINT8*)range[2][0];
        UINTN tail=(UINTN)sm+entry->TableMaximumSize;
        while(TRUE)
        {
            if((UINTN)sm+sizeof(SMBIOS_STRUCTURE)>tail)
            {
                panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 end-of-table (Type 127) not found within declared"
                    " table size.");
            }
            SMBIOS_STRUCTURE* stru=(SMBIOS_STRUCTURE*)sm;
            if(stru->Type!=SMBIOS_TYPE_END_OF_TABLE)
            {
                if(stru->Length<sizeof(SMBIOS_STRUCTURE))
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 structure at offset 0x%X has invalid length %u.",
                        (UINTN)sm-range[2][0],stru->Length);
                }
                if((UINTN)sm+stru->Length>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 structure at offset 0x%X formatted area exceeds"
                        " table size.",(UINTN)sm-range[2][0]);
                }
                sm=(UINT8*)((UINTN)sm+stru->Length);
                while(sm[0]!=0||sm[1]!=0)
                {
                    if((UINTN)sm+1>=tail)
                    {
                        panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 structure at offset 0x%X has unterminated"
                            " string-list.",(UINTN)stru-range[2][0]);
                    }
                    sm++;
                }
                sm=(UINT8*)((UINTN)sm+2);
            }
            else
            {
                range[2][1]=(UINTN)sm+stru->Length+2;
                break;
            }
        }
    }

    EFI_MEMORY_TYPE types[3]={EfiMaxMemoryType,EfiMaxMemoryType,EfiMaxMemoryType};
    EFI_MEMORY_DESCRIPTOR* memmap=NULL;
    UINTN map_size=0,desc_size,map_key;
    UINT32 desc_ver;

    status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&desc_size,&desc_ver);
    if(status!=EFI_BUFFER_TOO_SMALL)
    {
        panic(status,"Failed to retrieve UEFI Memory Map.");
    }
    memmap=pool_alloc(map_size);
    if(memmap==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate UEFI Memory Map from bootstrap pool.");
    }
    panic_if_error(gBS->GetMemoryMap(&map_size,memmap,&map_key,&desc_size,&desc_ver),
        "Failed to retrieve UEFI Memory Map.");
    if(desc_ver!=EFI_MEMORY_DESCRIPTOR_VERSION)
    {
        panic(EFI_DEVICE_ERROR,"UEFI Memory Map descriptor version mismatch.");
    }

    EFI_MEMORY_DESCRIPTOR* desc=memmap;
    UINTN end=map_size+(UINTN)memmap;
    while((UINTN)desc<end)
    {
        UINTN desc_end=EFI_PAGES_TO_SIZE(desc->NumberOfPages)+desc->PhysicalStart;
        for(UINTN index=0;index<3;index++)
        {
            if(desc->PhysicalStart<=range[index][0]&&range[index][1]<=desc_end)
            {
                if(types[index]>=EfiMaxMemoryType)
                {
                    types[index]=desc->Type;
                }
                else
                {
                    panic(EFI_DEVICE_ERROR,"UEFI Memory Map descriptor overlap detected.");
                }
            }
        }
        desc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)desc+desc_size);
    }
    pool_free(memmap);

    if(types[0]!=EfiACPIReclaimMemory)
    {
        panic(EFI_DEVICE_ERROR,"ACPI RSDP is not in ACPI Reclaim memory.");
    }
    if(!(types[1]==EfiMaxMemoryType||types[1]==EfiReservedMemoryType||types[1]==EfiRuntimeServicesData||
        types[1]==EfiRuntimeServicesCode)||!(types[2]==EfiMaxMemoryType||types[2]==EfiReservedMemoryType||
        types[2]==EfiRuntimeServicesData||types[2]==EfiRuntimeServicesCode))
    {
        UINTN size=range[1][1]-range[1][0]+range[2][1]-range[2][0];
        EFI_PHYSICAL_ADDRESS smaddr=SIZE_4GB;
        panic_if_error(gBS->AllocatePages(AllocateMaxAddress,EfiRuntimeServicesData,EFI_SIZE_TO_PAGES(size),&smaddr),
            "Failed to allocate memory for SMBIOS relocation.");
        if(is3)
        {
            SMBIOS_TABLE_3_0_ENTRY_POINT* copy=(SMBIOS_TABLE_3_0_ENTRY_POINT*)smaddr;
            SMBIOS_STRUCTURE* smcopy=(SMBIOS_STRUCTURE*)((UINTN)copy+sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT));
            CopyMem(copy,(VOID*)params->smbios,
                sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT));
            CopyMem(smcopy,(VOID*)copy->TableAddress,range[2][1]-range[2][0]);
            copy->TableAddress=(UINT64)smcopy;
            copy->EntryPointStructureChecksum=0;
            UINT8 checksum=0,index=0;
            UINT8* copydata=(UINT8*)copy;
            while(index<copy->EntryPointLength)
            {
                checksum+=copydata[index++];
            }
            copy->EntryPointStructureChecksum=-checksum;
            params->smbios=(UINTN)copy;
        }
        else
        {
            SMBIOS_TABLE_ENTRY_POINT* copy=(SMBIOS_TABLE_ENTRY_POINT*)smaddr;
            SMBIOS_STRUCTURE* smcopy=(SMBIOS_STRUCTURE*)((UINTN)copy+sizeof(SMBIOS_TABLE_ENTRY_POINT));
            CopyMem(copy,(VOID*)params->smbios,
                sizeof(SMBIOS_TABLE_ENTRY_POINT));
            CopyMem(smcopy,(VOID*)((UINTN)copy->TableAddress),
                range[2][1]-range[2][0]);
            copy->TableAddress=(UINT32)((UINTN)smcopy&MAX_UINT32);
            copy->EntryPointStructureChecksum=0;
            UINT8 checksum=0,index=0;
            UINT8* copydata=(UINT8*)copy;
            while(index<copy->EntryPointLength)
            {
                checksum+=copydata[index++];
            }
            copy->EntryPointStructureChecksum=-checksum;
            params->smbios=(UINTN)copy;
        }
    }
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
    /*关闭看门狗*/
    panic_if_error(gBS->SetWatchdogTimer(0,0,0,NULL),"Failed to disable watchdog.");

    /*引导内存池空间申请*/
    UINTN bpbase=init_bootstrap_pool();
    aos_boot_params* params=pool_alloc(sizeof(aos_boot_params));
    if(params==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"Failed to allocate AOS boot parameters from bootstrap pool.");
    }
    params->kminfo.bpbase=bpbase;

    /*CPU检查*/
    main_cpu_check(params);

    /*设置表*/
    main_set_table(params);

    panic_if_error(EFI_DEVICE_ERROR,"Hello?");
    return EFI_SUCCESS;
}
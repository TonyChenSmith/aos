/**
 * 模块引导代码。
 * @date 2026-06-27
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Guid/Acpi.h>
#include <Guid/SmBios.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/RngLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Register/Amd/ArchitecturalMsr.h>
#include <Register/Amd/Cpuid.h>
#include <Register/Intel/ArchitecturalMsr.h>
#include <Register/Intel/Cpuid.h>
#include <Register/Intel/LocalApic.h>

#include "config.h"
#include "idt.h"
#include "load.h"
#include "mark.h"
#include "page.h"
#include "panic.h"
#include "params.h"
#include "pool.h"
#include "verify.h"
#include "version.h"
#include "vma.h"

/**
 * 系统使用的固定GDT常量。
 */
CONST IA32_SEGMENT_DESCRIPTOR AOS_FIXED_GDT[]={
    {
        .Uint64=0x0000000000000000ULL,
    }, /*NULL*/
    {
        .Bits={
            .LimitLow=0xFFFF,
            .BaseLow=0,
            .BaseMid=0,
            .Type=0xB,
            .S=1,
            .DPL=0,
            .P=1,
            .LimitHigh=0xF,
            .AVL=0,
            .L=0,
            .DB=1,
            .G=1,
            .BaseHigh=0
        }
    }, /*KERNEL_CODE32*/
    {
        .Bits={
            .LimitLow=0xFFFF,
            .BaseLow=0,
            .BaseMid=0,
            .Type=0x3,
            .S=1,
            .DPL=0,
            .P=1,
            .LimitHigh=0xF,
            .AVL=0,
            .L=0,
            .DB=1,
            .G=1,
            .BaseHigh=0
        }
    }, /*KERNEL_DATA32*/
    {
        .Bits={
            .LimitLow=0xFFFF,
            .BaseLow=0,
            .BaseMid=0,
            .Type=0xB,
            .S=1,
            .DPL=0,
            .P=1,
            .LimitHigh=0xF,
            .AVL=0,
            .L=1,
            .DB=0,
            .G=1,
            .BaseHigh=0
        }
    }, /*KERNEL_CODE64*/
    {
        .Bits={
            .LimitLow=0xFFFF,
            .BaseLow=0,
            .BaseMid=0,
            .Type=0x3,
            .S=1,
            .DPL=0,
            .P=1,
            .LimitHigh=0xF,
            .AVL=0,
            .L=0,
            .DB=1,
            .G=1,
            .BaseHigh=0
        }
    }, /*KERNEL_DATA64*/
    {
        .Bits={
            .LimitLow=0xFFFF,
            .BaseLow=0,
            .BaseMid=0,
            .Type=0x3,
            .S=1,
            .DPL=3,
            .P=1,
            .LimitHigh=0xF,
            .AVL=0,
            .L=0,
            .DB=1,
            .G=1,
            .BaseHigh=0
        }
    }, /*USER_DATA64*/
    {
        .Bits={
            .LimitLow=0xFFFF,
            .BaseLow=0,
            .BaseMid=0,
            .Type=0xB,
            .S=1,
            .DPL=3,
            .P=1,
            .LimitHigh=0xF,
            .AVL=0,
            .L=1,
            .DB=0,
            .G=1,
            .BaseHigh=0
        }
    }, /*USER_CODE64*/
    {
        .Uint64=0x0000000000000000ULL,
    }, /*BOUNDARY*/
};

/**
 * 系统共享区单栈页数。
 */
CONST UINTN AOS_SHARED_STACK=2;

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
STATIC VOID EFIAPI boot_cpu_check(IN OUT aos_boot_params* params)
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
 * 设置启动参数所需系统表。
 * 
 * @param parmas 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_set_table(OUT aos_boot_params* params)
{
    UINTN rsdp_start,rsdp_end;
    VOID* table;
    panic_if_error(EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid,&table),
        "ACPI RSDP not found in UEFI Configuration Table. This system does not support ACPI, which is"
        " required.");
    params->acpi=(UINTN)table;

    EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp=(EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)params->acpi;
    if(rsdp->Signature!=EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE)
    {
        panic(EFI_DEVICE_ERROR,"ACPI RSDP signature mismatch.");
    }
    if(rsdp->Revision<EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION)
    {
        panic(EFI_UNSUPPORTED,"This system's ACPI revision is below 5.0, which is required to boot.");
    }
    rsdp_start=params->acpi;
    rsdp_end=params->acpi+rsdp->Length;

    EFI_MEMORY_DESCRIPTOR* memmap=NULL;
    UINTN map_size=0,desc_size,map_key;
    UINT32 desc_ver;

    EFI_STATUS status=gBS->GetMemoryMap(&map_size,memmap,&map_key,&desc_size,&desc_ver);
    if(status!=EFI_BUFFER_TOO_SMALL)
    {
        panic(status,"Failed to retrieve UEFI Memory Map.");
    }
    memmap=pool_alloc(map_size+3*desc_size);
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
    if(desc_size<sizeof(EFI_MEMORY_DESCRIPTOR))
    {
        panic(EFI_DEVICE_ERROR,"UEFI Memory Map descriptor size mismatch.");
    }

    EFI_MEMORY_DESCRIPTOR* desc=memmap;
    UINTN end=map_size+(UINTN)memmap;
    BOOLEAN found=FALSE;
    while((UINTN)desc<end)
    {
        UINTN desc_end=EFI_PAGES_TO_SIZE(desc->NumberOfPages)+desc->PhysicalStart;
        if(desc->PhysicalStart<=rsdp_start&&rsdp_end<=desc_end)
        {
            if(desc->Type!=EfiACPIReclaimMemory)
            {
                panic(EFI_DEVICE_ERROR,"ACPI RSDP is not in ACPI Reclaim memory.");
            }
            found=TRUE;
        }
        desc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)desc+desc_size);
    }
    if(!found)
    {
        panic(EFI_DEVICE_ERROR,"ACPI RSDP not found in UEFI memory map.");
    }
    pool_free(memmap);

    status=EfiGetSystemConfigurationTable(&gEfiSmbios3TableGuid,&table);
    if(EFI_ERROR(status))
    {
        panic_if_error(EfiGetSystemConfigurationTable(&gEfiSmbiosTableGuid,&table),
            "SMBIOS not found in UEFI Configuration Table. This system does not expose SMBIOS,"
            " which is required.");
        UINTN smbios_entry_base=(UINTN)table;
        SMBIOS_TABLE_ENTRY_POINT* entry=(SMBIOS_TABLE_ENTRY_POINT*)table;
        UINTN smbios_base=entry->TableAddress;
        UINTN smbios_size;

        UINT8 checksum=0,index=0;
        UINT8* edata=(UINT8*)smbios_entry_base;
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

        UINT8* sm=(UINT8*)smbios_base;
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
                        (UINTN)sm-smbios_base,stru->Length);
                }
                if((UINTN)sm+stru->Length>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X formatted area exceeds table"
                        " size.",(UINTN)sm-smbios_base);
                }
                sm=(UINT8*)((UINTN)sm+stru->Length);
                if((UINTN)sm+2>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X missing string-list"
                        " terminator.",(UINTN)sm-smbios_base);
                }
                while(sm[0]!=0||sm[1]!=0)
                {
                    if((UINTN)sm+1>=tail)
                    {
                        panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X has unterminated"
                            " string-list.",(UINTN)stru-smbios_base);
                    }
                    sm++;
                }
                sm=(UINT8*)((UINTN)sm+2);
            }
            else
            {
                if(stru->Length<sizeof(SMBIOS_STRUCTURE))
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X has invalid length %u.",
                        (UINTN)sm-smbios_base,stru->Length);
                }
                sm=(UINT8*)((UINTN)sm+stru->Length);
                if((UINTN)sm+2>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X missing string-list"
                        " terminator.",(UINTN)sm-smbios_base);
                }
                smbios_size=(UINTN)sm+2-smbios_base;
                break;
            }
        }

        SMBIOS_TABLE_ENTRY_POINT* copy=(SMBIOS_TABLE_ENTRY_POINT*)allocate_page(AOS_BOOT_MEMORY_SMBIOS,
            EFI_SIZE_TO_PAGES(sizeof(SMBIOS_TABLE_ENTRY_POINT)+smbios_size),SIZE_4GB-1);
        SMBIOS_STRUCTURE* smcopy=(SMBIOS_STRUCTURE*)((UINTN)copy+sizeof(SMBIOS_TABLE_ENTRY_POINT));
        CopyMem(copy,(VOID*)smbios_entry_base,sizeof(SMBIOS_TABLE_ENTRY_POINT));
        CopyMem(smcopy,(VOID*)((UINTN)copy->TableAddress),smbios_size);
        copy->TableAddress=(UINT32)((UINTN)smcopy&MAX_UINT32);
        copy->EntryPointStructureChecksum=0;
        checksum=0,index=0;
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
        UINTN smbios_entry_base=(UINTN)table;
        SMBIOS_TABLE_3_0_ENTRY_POINT* entry=(SMBIOS_TABLE_3_0_ENTRY_POINT*)table;
        UINTN smbios_base=entry->TableAddress;
        UINTN smbios_size;

        UINT8 checksum=0,index=0;
        UINT8* edata=(UINT8*)smbios_entry_base;
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

        UINT8* sm=(UINT8*)smbios_base;
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
                        (UINTN)sm-smbios_base,stru->Length);
                }
                if((UINTN)sm+stru->Length>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 structure at offset 0x%X formatted area exceeds"
                        " table size.",(UINTN)sm-smbios_base);
                }
                sm=(UINT8*)((UINTN)sm+stru->Length);
                if((UINTN)sm+2>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X missing string-list"
                        " terminator.",(UINTN)sm-smbios_base);
                }
                while(sm[0]!=0||sm[1]!=0)
                {
                    if((UINTN)sm+1>=tail)
                    {
                        panic(EFI_DEVICE_ERROR,"SMBIOS 3.0 structure at offset 0x%X has unterminated"
                            " string-list.",(UINTN)stru-smbios_base);
                    }
                    sm++;
                }
                sm=(UINT8*)((UINTN)sm+2);
            }
            else
            {
                if(stru->Length<sizeof(SMBIOS_STRUCTURE))
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X has invalid length %u.",
                        (UINTN)sm-smbios_base,stru->Length);
                }
                sm=(UINT8*)((UINTN)sm+stru->Length);
                if((UINTN)sm+2>tail)
                {
                    panic(EFI_DEVICE_ERROR,"SMBIOS structure at offset 0x%X missing string-list"
                        " terminator.",(UINTN)sm-smbios_base);
                }
                smbios_size=(UINTN)sm+2-smbios_base;
                break;
            }
        }

        SMBIOS_TABLE_3_0_ENTRY_POINT* copy=(SMBIOS_TABLE_3_0_ENTRY_POINT*)allocate_page(AOS_BOOT_MEMORY_SMBIOS,
            EFI_SIZE_TO_PAGES(sizeof(SMBIOS_TABLE_ENTRY_POINT)+smbios_size),SIZE_4GB-1);
        SMBIOS_STRUCTURE* smcopy=(SMBIOS_STRUCTURE*)((UINTN)copy+sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT));
        CopyMem(copy,(VOID*)smbios_entry_base,
            sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT));
        CopyMem(smcopy,(VOID*)copy->TableAddress,smbios_size);
        copy->TableAddress=(UINT64)smcopy;
        copy->EntryPointStructureChecksum=0;
        checksum=0,index=0;
        UINT8* copydata=(UINT8*)copy;
        while(index<copy->EntryPointLength)
        {
            checksum+=copydata[index++];
        }
        copy->EntryPointStructureChecksum=-checksum;
        params->smbios=(UINTN)copy;
    }
}

/**
 * 查找CPU。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_find_cpus(IN OUT aos_boot_params* params)
{
    /*统计CPU槽数*/
    typedef struct _cpu_data
    {
        struct _cpu_data* next;  /*下一指针。*/
        UINT32            apic;  /*APIC ID。*/
        UINT32            flags; /*标志。*/
    } cpu_data;

    UINT32 bsp=0;
    if(params->state.apic==AOS_APIC_NO_APIC)
    {
        UINT32 max_leaf=0,ebx=0;

        AsmCpuid(CPUID_SIGNATURE,&max_leaf,NULL,NULL,NULL);
        if(max_leaf>=CPUID_EXTENDED_TOPOLOGY)
        {
            AsmCpuidEx(CPUID_EXTENDED_TOPOLOGY,0,NULL,&ebx,NULL,&bsp);
        }
        if((max_leaf<CPUID_EXTENDED_TOPOLOGY)||((ebx&0xFFFF)==0))
        {
            AsmCpuid(CPUID_VERSION_INFO,NULL,&ebx,NULL,NULL);
            bsp=ebx>>24;
        }
    }
    else if(params->state.apic==AOS_APIC_X2APIC)
    {
        bsp=AsmReadMsr32(MSR_IA32_X2APIC_APICID);
    }
    else
    {
        MSR_IA32_APIC_BASE_REGISTER apic_base;
        apic_base.Uint64=AsmReadMsr64(MSR_IA32_APIC_BASE);
        UINT64 base=((UINT64)apic_base.Bits.ApicBase<<12)|((UINT64)apic_base.Bits.ApicBaseHi<<32);
        bsp=MmioRead32(base+XAPIC_ID_OFFSET)>>24;
    }

    cpu_data* xapic=NULL;
    UINTN xapics=0;
    cpu_data* x2apic=NULL;
    UINTN x2apics=0;
    BOOLEAN xapic_bsp=FALSE;
    BOOLEAN x2apic_bsp=FALSE;

    EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp=(EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)params->acpi;
    EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* madt=NULL;
    if(rsdp->XsdtAddress)
    {
        EFI_ACPI_DESCRIPTION_HEADER* xsdt=(EFI_ACPI_DESCRIPTION_HEADER*)rsdp->XsdtAddress;
        UINTN count=(xsdt->Length-sizeof(EFI_ACPI_DESCRIPTION_HEADER))>>3;
        UINT64* tables=(UINT64*)((UINTN)xsdt+sizeof(EFI_ACPI_DESCRIPTION_HEADER));
        for(UINTN index=0;index<count;index++)
        {
            if(((EFI_ACPI_DESCRIPTION_HEADER*)tables[index])->Signature==
                EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE)
            {
                madt=(EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)tables[index];
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
            if(((EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)tables[index])->Signature==
                EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE)
            {
                madt=(EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)(UINTN)tables[index];
                break;
            }
        }
    }

    if(madt==NULL)
    {
        panic(EFI_UNSUPPORTED,"MADT not found.");
    }

    UINT8* p=(UINT8*)(madt+1);
    UINT8* end=(UINT8*)madt+madt->Header.Length;

    while(p+2<=end)
    {
        UINT8 type=p[0];
        UINT8 len=p[1];

        if(len<2||p+len>end)
        {
            panic(EFI_DEVICE_ERROR,"Malformed MADT entry.");
        }

        if(type==EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC)
        {
            EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_STRUCTURE* lapic=(EFI_ACPI_5_0_PROCESSOR_LOCAL_APIC_STRUCTURE*)p;
            if(lapic->ApicId==MAX_UINT8)
            {
                p+=len;
                continue;
            }
            BOOLEAN new_apic=TRUE;
            cpu_data* prev=NULL;
            cpu_data* node=xapic;
            while(node!=NULL)
            {
                if(node->apic==lapic->ApicId)
                {
                    new_apic=FALSE;
                    break;
                }
                prev=node;
                node=node->next;
            }
            if(new_apic)
            {
                if(lapic->ApicId==bsp)
                {
                    xapic_bsp=TRUE;
                }
                node=pool_alloc(sizeof(cpu_data));
                if(node==NULL)
                {
                    panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool.");
                }
                node->apic=lapic->ApicId;
                node->flags=lapic->Flags;
                node->next=NULL;
                xapics++;
                if(prev==NULL)
                {
                    xapic=node;
                }
                else
                {
                    prev->next=node;
                }
            }
        }
        p+=len;
    }

    p=(UINT8*)(madt+1);
    while(p+2<=end)
    {
        UINT8 type=p[0];
        UINT8 len=p[1];

        if(len<2||p+len>end)
        {
            panic(EFI_DEVICE_ERROR,"Malformed MADT entry.");
        }

        if(type==EFI_ACPI_5_0_PROCESSOR_LOCAL_X2APIC)
        {
            EFI_ACPI_5_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE* lapic=(EFI_ACPI_5_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE*)p;
            if(lapic->X2ApicId==MAX_UINT32)
            {
                p+=len;
                continue;
            }
            if(xapics>0&&lapic->X2ApicId<MAX_UINT8)
            {
                panic(EFI_DEVICE_ERROR,"MADT contains both xAPIC and x2APIC entries, and x2APIC ID"
                    " 0x%X overlaps the 8-bit APIC ID range.",(UINTN)lapic->X2ApicId);
            }
            BOOLEAN new_apic=TRUE;
            cpu_data* prev=NULL;
            cpu_data* node=x2apic;
            while(node!=NULL)
            {
                if(node->apic==lapic->X2ApicId)
                {
                    new_apic=FALSE;
                    break;
                }
                prev=node;
                node=node->next;
            }
            if(new_apic)
            {
                if(lapic->X2ApicId==bsp)
                {
                    x2apic_bsp=TRUE;
                }
                node=pool_alloc(sizeof(cpu_data));
                if(node==NULL)
                {
                    panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool.");
                }
                node->apic=lapic->X2ApicId;
                node->flags=lapic->Flags;
                node->next=NULL;
                x2apics++;
                if(prev==NULL)
                {
                    x2apic=node;
                }
                else
                {
                    prev->next=node;
                }
            }
        }
        p+=len;
    }

    if(!(xapic_bsp||x2apic_bsp))
    {
        panic(EFI_DEVICE_ERROR,"BSP APIC ID 0x%X not found in MADT.",(UINTN)bsp);
    }

    params->cpu_count=xapics+x2apics;
    params->cpus=pool_alloc(sizeof(UINT32)*params->cpu_count);
    if(params->cpus==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool.");
    }
    params->cpus[0]=bsp;

    UINTN cpu_index=1;
    cpu_data* node=xapic;
    while(node!=NULL)
    {
        if(node->apic!=bsp&&(node->flags&EFI_ACPI_5_0_LOCAL_APIC_ENABLED))
        {
            params->cpus[cpu_index]=node->apic;
            cpu_index++;
        }
        node=node->next;
    }

    node=x2apic;
    while(node!=NULL)
    {
        if(node->apic!=bsp&&(node->flags&EFI_ACPI_5_0_LOCAL_APIC_ENABLED))
        {
            params->cpus[cpu_index]=node->apic;
            cpu_index++;
        }
        node=node->next;
    }

    params->enabled_count=cpu_index;
    if(cpu_index<params->cpu_count)
    {
        UINTN disable_count=params->cpu_count-cpu_index;
        UINT32* disable=pool_alloc(sizeof(UINT32)*disable_count);
        if(disable==NULL)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool.");
        }

        UINTN disable_index=0;
        node=xapic;
        while(node!=NULL)
        {
            if(node->apic!=bsp&&!(node->flags&EFI_ACPI_5_0_LOCAL_APIC_ENABLED))
            {
                disable[disable_index]=node->apic;
                disable_index++;
            }
            node=node->next;
        }

        node=x2apic;
        while(node!=NULL)
        {
            if(node->apic!=bsp&&!(node->flags&EFI_ACPI_5_0_LOCAL_APIC_ENABLED))
            {
                disable[disable_index]=node->apic;
                disable_index++;
            }
            node=node->next;
        }

        for(UINTN di=0;di<disable_count-1;di++)
        {
            UINTN dmin=di;
            for(UINTN dj=di+1;dj<disable_count;dj++)
            {
                if(disable[dj]<disable[dmin])
                {
                    dmin=dj;
                }
            }

            if(dmin!=di)
            {
                UINT32 tmp=disable[di];
                disable[di]=disable[dmin];
                disable[dmin]=tmp;
            }
        }

        for(UINTN di=0;di<disable_count;di++)
        {
            params->cpus[cpu_index]=disable[di];
            cpu_index++;
        }
        pool_free(disable);
    }

    cpu_data* node_next=NULL;
    node=xapic;
    while(node!=NULL)
    {
        node_next=node->next;
        pool_free(node);
        node=node_next;
    }

    node_next=NULL;
    node=x2apic;
    while(node!=NULL)
    {
        node_next=node->next;
        pool_free(node);
        node=node_next;
    }
}

/**
 * 生成随机64位数。参数通过指针传递，方便后续二次加强计算。
 * 
 * @param a 参数A。
 * @param b 参数B。
 * @param c 参数C。
 * 
 * @return 返回一个随机数。
 */
STATIC UINT64 EFIAPI boot_random(IN OUT UINT64* a,IN OUT UINT64* b,IN OUT UINT64* c)
{
    UINT64 ra,rb,rc;
    UINT64 fresh;

    RandomBytes((UINT8*)&ra,sizeof(ra));
    RandomBytes((UINT8*)&rb,sizeof(rb));
    RandomBytes((UINT8*)&rc,sizeof(rc));
    GetRandomNumber64(&fresh);

    *a^=ra^fresh;
    *b^=rb^LRotU64(fresh,23);
    *c^=rc^LRotU64(fresh,47);

    UINT64 value=*a;
    value^=LRotU64(*b,17);
    value^=*c;
    value=MultU64x64(value,0x9E3779B97F4A7C15ULL);
    value^=value>>30;
    value=MultU64x64(value,0xBF58476D1CE4E5B9ULL);
    value^=value>>27;
    value=MultU64x64(value,0x94D049BB133111EBULL);
    value^=value>>31;

    return value;
}

/**
 * 共享区域基址。
 */
STATIC UINTN share_base=-SIZE_512GB;

/**
 * 固件基址。
 */
STATIC UINTN firmware_base=-2*SIZE_512GB;

/**
 * 内核基址。
 */
STATIC UINTN kernel_base=-3*SIZE_512GB;

/**
 * 内核数据基址。
 */
STATIC UINTN data_base=-(3*SIZE_512GB-SIZE_128GB);

/**
 * 内核栈基址。
 */
STATIC UINTN stack_base=-(3*SIZE_512GB-SIZE_256GB);

/**
 * 准备基址。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_prepare_base(VOID)
{
    if(!CONFIG_RANDOMIZE_BASE)
    {
        firmware_base+=SIZE_4GB;
        kernel_base+=SIZE_2GB;
        data_base+=SIZE_2GB;
        stack_base+=SIZE_2GB;
        return;
    }

    UINT64 a=AOS_UEFI_VERSION,b=AOS_UEFI_VERSION_0_0_1,c=AOS_UEFI_VERSION_0_1_3;
    UINT64 random=boot_random(&a,&b,&c);
    UINTN min_base=firmware_base+SIZE_4GB;
    UINTN max_base=firmware_base+SIZE_512GB-SIZE_8GB;
    UINTN slots=(max_base-min_base)/SIZE_2MB;
    UINTN offset=(UINTN)(random%slots);
    firmware_base=min_base+offset*SIZE_2MB;

    random=boot_random(&a,&b,&c);
    min_base=kernel_base+SIZE_2GB;
    max_base=kernel_base+SIZE_128GB-SIZE_2GB-SIZE_4GB;
    slots=(max_base-min_base)/SIZE_2MB;
    offset=(UINTN)(random%slots);
    kernel_base=min_base+offset*SIZE_2MB;

    random=boot_random(&a,&b,&c);
    min_base=data_base+SIZE_2GB;
    max_base=data_base+SIZE_128GB-SIZE_2GB-SIZE_4GB;
    slots=(max_base-min_base)/SIZE_2MB;
    offset=(UINTN)(random%slots);
    data_base=min_base+offset*SIZE_2MB;

    random=boot_random(&a,&b,&c);
    min_base=stack_base+SIZE_2GB;
    max_base=stack_base+SIZE_128GB-SIZE_2GB-EFI_PAGES_TO_SIZE(CONFIG_KERNEL_STACK);
    slots=(max_base-min_base)/SIZE_2MB;
    offset=(UINTN)(random%slots);
    stack_base=min_base+offset*SIZE_2MB;
}

/**
 * 准备CPU运行数据。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_prepare_cpu_data(IN aos_boot_params* params)
{
    UINTN kernel_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,CONFIG_KERNEL_STACK,SIZE_4GB-1);
    if(kernel_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the kernel stack space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,stack_base,kernel_stack,CONFIG_KERNEL_STACK,
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|AOS_BOOT_VMA_ALLOCATED);

    UINTN gdt_size=MAX(sizeof(AOS_FIXED_GDT)+sizeof(IA32_TSS_DESCRIPTOR),128)+MAX(sizeof(IA32_TASK_STATE_SEGMENT),128);
    UINTN gdt_pages=EFI_SIZE_TO_PAGES(gdt_size);
    UINT64* gdt=allocate_page(AOS_BOOT_MEMORY_GDT,gdt_pages,SIZE_4GB-1);
    if(gdt==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the BSP GDT space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_GDT,data_base+(UINTN)gdt,(UINTN)gdt,gdt_pages,
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_UC|AOS_BOOT_VMA_ALLOCATED);
    add_vma(AOS_BOOT_VMA_TYPE_GDT,share_base,(UINTN)gdt,gdt_pages,
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_CACHE_UC|AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);

    ZeroMem(gdt,EFI_PAGES_TO_SIZE(gdt_pages));
    UINTN index;
    for(index=0;index<ARRAY_SIZE(AOS_FIXED_GDT);index++)
    {
        gdt[index]=AOS_FIXED_GDT[index].Uint64;
    }
    IA32_TASK_STATE_SEGMENT* tss=(IA32_TASK_STATE_SEGMENT*)((UINTN)gdt+MAX(sizeof(AOS_FIXED_GDT)
        +sizeof(IA32_TSS_DESCRIPTOR),128));
    UINTN tss_base=share_base+MAX(sizeof(AOS_FIXED_GDT)+sizeof(IA32_TSS_DESCRIPTOR),128);
    IA32_TSS_DESCRIPTOR* tss_desc=(IA32_TSS_DESCRIPTOR*)&gdt[index];
    tss_desc->Uint128.Uint64=0;
    tss_desc->Uint128.Uint64_1=0;
    tss_desc->Bits.LimitLow=sizeof(IA32_TASK_STATE_SEGMENT)-1;
    tss_desc->Bits.BaseLow=tss_base&0xFFFF;
    tss_desc->Bits.BaseMidl=(tss_base>>16)&0xFF;
    tss_desc->Bits.Type=IA32_GDT_TYPE_TSS;
    tss_desc->Bits.DPL=0;
    tss_desc->Bits.P=1;
    tss_desc->Bits.LimitHigh=0;
    tss_desc->Bits.AVL=0;
    tss_desc->Bits.G=0;
    tss_desc->Bits.BaseMidh=(tss_base>>24)&0xFF;
    tss_desc->Bits.BaseHigh=(tss_base>>32)&0xFFFFFFFF;
    ZeroMem(tss,sizeof(IA32_TASK_STATE_SEGMENT));
    tss->IOMapBaseAddress=sizeof(IA32_TASK_STATE_SEGMENT);

    UINTN rsp0_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(rsp0_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS RSP0 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_4MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        rsp0_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->RSP0=share_base+SIZE_4MB;

    UINTN ist1_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist1_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST1 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_4MB+SIZE_2MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist1_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[0]=share_base+SIZE_4MB+SIZE_2MB;

    UINTN ist2_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist2_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST2 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_8MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist2_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[1]=share_base+SIZE_8MB;

    UINTN ist3_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist3_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST3 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_8MB+SIZE_2MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist3_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[2]=share_base+SIZE_8MB+SIZE_2MB;

    UINTN ist4_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist4_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST4 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_8MB+SIZE_4MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist4_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[3]=share_base+SIZE_8MB+SIZE_4MB;

    UINTN ist5_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist5_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST5 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_8MB+SIZE_4MB+SIZE_2MB-
        EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),ist5_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|
        AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[4]=share_base+SIZE_8MB+SIZE_4MB+SIZE_2MB;

    UINTN ist6_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist6_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST6 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_16MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist6_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[5]=share_base+SIZE_16MB;

    UINTN ist7_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist7_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST7 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_STACK,share_base+SIZE_16MB+SIZE_2MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist7_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[6]=share_base+SIZE_16MB+SIZE_2MB;

    UINTN idt_handler_pages=EFI_SIZE_TO_PAGES(AOS_BOOT_IDT_DATA_SIZE);
    UINT8* idt_handler=allocate_page(AOS_BOOT_MEMORY_FIRMWARE,idt_handler_pages,SIZE_4GB-1);
    if(idt_handler==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the IDT handler space.");
    }
    SetMem(idt_handler,EFI_PAGES_TO_SIZE(idt_handler_pages),0xCC);
    CopyMem(idt_handler,AOS_BOOT_IDT_DATA,AOS_BOOT_IDT_DATA_SIZE);
    UINTN idt_handler_base=firmware_base+(UINTN)idt_handler;
    add_vma(AOS_BOOT_VMA_TYPE_FIRMWARE,idt_handler_base,(UINTN)idt_handler,
        idt_handler_pages,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_EXECUTE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_ALLOCATED);

    UINTN idt_size=sizeof(IA32_IDT_GATE_DESCRIPTOR)*256;
    UINTN idt_pages=EFI_SIZE_TO_PAGES(idt_size);
    IA32_IDT_GATE_DESCRIPTOR* idt=allocate_page(AOS_BOOT_MEMORY_IDT,idt_pages,SIZE_4GB-1);
    if(idt==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the IDT space.");
    }
    ZeroMem(idt,idt_size);
    add_vma(AOS_BOOT_VMA_TYPE_IDT,data_base+(UINTN)idt,(UINTN)idt,idt_pages,
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_UC|AOS_BOOT_VMA_ALLOCATED);
    add_vma(AOS_BOOT_VMA_TYPE_IDT,share_base+SIZE_8KB,(UINTN)idt,idt_pages,
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_CACHE_UC|AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    
    for(UINTN index=0;index<AOS_IDT_VECTOR_COUNT;index++)
    {
        idt[index].Bits.GateType=IA32_IDT_GATE_TYPE_INTERRUPT_32;
        idt[index].Bits.Selector=3*sizeof(IA32_SEGMENT_DESCRIPTOR);
        UINTN offset=idt_handler_base+AOS_BOOT_IDT_HANDLER_OFFSET[index];
        idt[index].Bits.OffsetLow=offset&0xFFFF;
        idt[index].Bits.OffsetHigh=(offset>>16)&0xFFFF;
        idt[index].Bits.OffsetUpper=(offset>>32)&0xFFFFFFFF;
    }
}

/**
 * 查询AOS系统卷类型。
 * 
 * @param params     启动参数。
 * @param identifier 识别符指针。
 * 
 * @return AOS系统卷类型。
 */
STATIC asv_type EFIAPI boot_find_asv(IN OUT aos_boot_params* params,OUT VOID** identifier)
{
    if(params==NULL)
    {
        return ASV_ESP;
    }

    init_esp_io(params);
    esp_mount();

    EFI_FILE_HANDLE asv_file=esp_open(L"\\aos\\aos.system.volume",EFI_FILE_MODE_READ,0);
    if(asv_file==NULL)
    {
        esp_unmount();
        return ASV_ESP;
    }

    UINT64 asv_size=esp_get_size(asv_file);
    UINTN read_size=asv_size;
    CHAR8* asv=pool_alloc(asv_size+1);
    if(asv==NULL)
    {
        asv_file->Close(asv_file);
        esp_unmount();
        return ASV_ESP;
    }
    ZeroMem(asv,asv_size+1);

    EFI_STATUS status=asv_file->Read(asv_file,&read_size,asv);
    asv_file->Close(asv_file);
    if(EFI_ERROR(status)||read_size!=asv_size)
    {
        pool_free(asv);
        esp_unmount();
        return ASV_ESP;
    }

    esp_unmount();

    CHAR8* node=asv;
    CHAR8* sep=AsciiStrStr(node,",");
    if(sep==NULL)
    {
        /*无论是否为ESP字符串，都认为是ESP*/
        pool_free(asv);
        return ASV_ESP;
    }
    *sep=0;

    if(AsciiStrCmp(node,"HD")==0)
    {
        node=sep+1;
        sep=AsciiStrStr(node,",");
        if(sep==NULL)
        {
            pool_free(asv);
            return ASV_ESP;
        }
        *sep=0;

        if(AsciiStrCmp(node,"GPT")==0)
        {
            node=sep+1;
            if(AsciiStrSize(node)!=37)
            {
                pool_free(asv);
                return ASV_ESP;
            }
            
            EFI_GUID* guid=pool_alloc(sizeof(EFI_GUID));
            if(guid==NULL)
            {
                pool_free(asv);
                return ASV_ESP;
            }

            status=AsciiStrToGuid(node,guid);
            if(EFI_ERROR(status))
            {
                pool_free(guid);
                pool_free(asv);
                return ASV_ESP;
            }
            
            pool_free(asv);
            *identifier=guid;
            return ASV_GPT;
        }
    }

    pool_free(asv);
    return ASV_ESP;
}

/**
 * 准备加载内核。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_verify_and_load(IN OUT aos_boot_params* params)
{
    VOID* identifier=NULL;
    asv_type asv=boot_find_asv(params,&identifier);

    init_asv_io(asv,identifier,params);

    asv_mount();

    asv_file* signature=asv_open("/aos/aos.signature");
    if(signature==NULL)
    {
        panic(EFI_NOT_FOUND,"The signature file is not found.");
    }

    asv_file* kernel=asv_open("/aos/aos.kernel");
    if(kernel==NULL)
    {
        panic(EFI_NOT_FOUND,"The kernel file is not found.");
    }

    verify_kernel(signature,kernel);
    UINTN entry=load_kernel(kernel,kernel_base);

    params->memory.kernel_entry=kernel_base+entry;
}

/**
 * 随机种子字符串。
 */
STATIC CHAR8 seed[512];

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
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate AOS boot parameters from bootstrap pool.");
    }

    /*CPU检查*/
    boot_cpu_check(params);

    /*设置系统表*/
    boot_set_table(params);

    /*查找CPU*/
    boot_find_cpus(params);

    /*初始化页表*/
    init_page_table_pool(params);

    /*设置随机种子*/
    UINT64 seed_value=AOS_UEFI_VERSION;
    GetRandomNumber64(&seed_value);
    AsciiSPrint(seed,sizeof(seed),"Module \"aos.uefi\" seed: "
        "The version bcd of the module \"aos.uefi\" is %016lX. The time stamp counter is %016lX. The random seed value "
        "is %016lX.",(UINT64)AOS_UEFI_VERSION,AsmReadTsc(),seed_value);
    RandomSeed((UINT8*)seed,sizeof(seed));

    /*准备基址*/
    boot_prepare_base();

    boot_prepare_cpu_data(params);

    boot_verify_and_load(params);

    UINTN EFIAPI page_table_pool_base(VOID);
    aos_boot_vma* EFIAPI vma_head(VOID);
    aos_boot_memory* EFIAPI memory_head(VOID);


    /*统计页表池位图占用情况*/
    UINTN pt_pages=params->memory.pt_pool_pages;
    CONST UINT8* pt_map=(CONST UINT8*)params->memory.pt_pool_map;
    UINTN pt_used=0;
    UINTN pt_first_free=pt_pages; /*首个空闲位；池满时等于页数*/
    for(UINTN i=0;i<pt_pages;i++)
    {
        if(pt_map[i>>3]&(UINT8)(1U<<(i&7)))
        {
            pt_used++;
        }
        else if(pt_first_free==pt_pages)
        {
            pt_first_free=i;
        }
    }

    CHAR8 report[768];
    AsciiSPrint(report,sizeof(report),
        "pool=0x%lX\n"
        "features: magic=0x%02X baseline=%u vendor=%u bits=0x%02X\n"
        "state: magic=0x%02X state=0x%02X vmtrr=%u apic=%u\n"
        "table: acpi=0x%lX smbios=0x%lX\n"
        "cpus: total=%lu enabled=%lu\n"
        "pt: base=0x%lX root=0x%lX pages=%lu bitmap=0x%lX used=%lu first_free=%lu\n"
        "size: params=%lu kinfo=%lu vma=%lu mem=%lu\n"
        "base: share=0x%lX fw=0x%lX kern=0x%lX data=0x%lX stack=0x%lX\n",
        (UINT64)bpbase,
        (UINTN)params->features.magic,(UINTN)params->features.baseline,
        (UINTN)params->features.vendor,(UINTN)params->features.features,
        (UINTN)params->state.magic,(UINTN)params->state.state,
        (UINTN)params->state.vmtrr,(UINTN)params->state.apic,
        (UINT64)params->acpi,(UINT64)params->smbios,
        (UINT64)params->cpu_count,(UINT64)params->enabled_count,
        (UINT64)page_table_pool_base(),(UINT64)params->memory.page_table,
        (UINT64)pt_pages,(UINT64)params->memory.pt_pool_map,
        (UINT64)pt_used,(UINT64)pt_first_free,
        (UINT64)sizeof(aos_boot_params),(UINT64)sizeof(aos_kernel_memory_info),
        (UINT64)sizeof(aos_boot_vma),(UINT64)sizeof(aos_boot_memory),
        (UINT64)share_base,(UINT64)firmware_base,(UINT64)kernel_base,
        (UINT64)data_base,(UINT64)stack_base);
    for(UINTN i=0;report[i]!=0;i++)
    {
        IoWrite8(0x402,(UINT8)report[i]);
    }

    /*CPU列表(有多少打多少)*/
    for(UINTN i=0;i<params->cpu_count;i++)
    {
        AsciiSPrint(report,sizeof(report),"cpu[%lu]=%u\n",(UINT64)i,(UINTN)params->cpus[i]);
        for(UINTN j=0;report[j]!=0;j++)
        {
            IoWrite8(0x402,(UINT8)report[j]);
        }
    }

    /*内存图输出*/
    aos_boot_memory* mem_node=memory_head();
    for(UINTN i=0;mem_node!=NULL;i++)
    {
        AsciiSPrint(report,sizeof(report),"mem[%lu] start=0x%lX size=0x%lX type=%lu\n",
            (UINT64)i,(UINT64)mem_node->start,(UINT64)mem_node->size,(UINT64)mem_node->type);
        for(UINTN j=0;report[j]!=0;j++)
        {
            IoWrite8(0x402,(UINT8)report[j]);
        }
        mem_node=mem_node->next;
    }

    /*线性区输出*/
    aos_boot_vma* vma_node=vma_head();
    for(UINTN i=0;vma_node!=NULL;i++)
    {
        AsciiSPrint(report,sizeof(report),"vma[%lu] start=0x%lX end=0x%lX flags=0x%lX type=%lu\n",
            (UINT64)i,(UINT64)vma_node->start,(UINT64)vma_node->end,
            (UINT64)vma_node->flags,(UINT64)vma_node->type);
        for(UINTN j=0;report[j]!=0;j++)
        {
            IoWrite8(0x402,(UINT8)report[j]);
        }
        vma_node=vma_node->next;
    }
    panic_if_error(EFI_DEVICE_ERROR,"Hello? %u",bpbase);
    return EFI_SUCCESS;
}
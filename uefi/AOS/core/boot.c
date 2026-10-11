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
#include <Library/CpuLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/RngLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>
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

/**
 * 内核蹦床函数，在换栈后进入内核入口函数不再返回。
 * 
 * @param params 启动参数。
 * @param stack  栈底。
 * 
 * @return 不再返回。
 */
typedef VOID NORETURN EFIAPI (*aos_kernel_trampoline)(IN aos_boot_params* params,IN UINTN stack);

/**
 * VGA水平分辨率。
 */
#define BOOT_VGA_HORIZONTAL_RESOLUTION 640

/**
 * VGA竖直分辨率。
 */
#define BOOT_VGA_VERTICAL_RESOLUTION 480

/**
 * SVGA水平分辨率。
 */
#define BOOT_SVGA_HORIZONTAL_RESOLUTION 800

/**
 * SVGA竖直分辨率。
 */
#define BOOT_SVGA_VERTICAL_RESOLUTION 600

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
 * @param params 启动参数。
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
    CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_EBX ml7ebx={.Uint32=ebx};
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
    if(ml7ebx.Bits.SMAP)
    {
        params->features.features|=AOS_FEATURES_SMAP;
    }
    if(ml7ebx.Bits.SMEP)
    {
        params->features.features|=AOS_FEATURES_SMEP;
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
 * @param params 启动参数。
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
 * 内核GDT物理基址。
 */
STATIC UINTN kernel_gdt=0;

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
    kernel_gdt=(UINTN)gdt;
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

    UINTN rsp0_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(rsp0_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS RSP0 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_4MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        rsp0_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->RSP0=share_base+SIZE_4MB;

    UINTN ist1_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist1_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST1 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_4MB+SIZE_2MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist1_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[0]=share_base+SIZE_4MB+SIZE_2MB;

    UINTN ist2_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist2_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST2 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_8MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist2_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[1]=share_base+SIZE_8MB;

    UINTN ist3_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist3_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST3 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_8MB+SIZE_2MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist3_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[2]=share_base+SIZE_8MB+SIZE_2MB;

    UINTN ist4_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist4_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST4 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_8MB+SIZE_4MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist4_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[3]=share_base+SIZE_8MB+SIZE_4MB;

    UINTN ist5_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist5_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST5 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_8MB+SIZE_4MB+SIZE_2MB-
        EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),ist5_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|
        AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[4]=share_base+SIZE_8MB+SIZE_4MB+SIZE_2MB;

    UINTN ist6_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist6_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST6 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_16MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
        ist6_stack,AOS_SHARED_STACK,AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_CACHE_WB|
        AOS_BOOT_VMA_GLOBAL|AOS_BOOT_VMA_ALLOCATED);
    tss->IST[5]=share_base+SIZE_16MB;

    UINTN ist7_stack=(UINTN)allocate_page(AOS_BOOT_MEMORY_FIXED_STACK,AOS_SHARED_STACK,SIZE_4GB-1);
    if(ist7_stack==0)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the TSS IST7 space.");
    }
    add_vma(AOS_BOOT_VMA_TYPE_FIXED_STACK,share_base+SIZE_16MB+SIZE_2MB-EFI_PAGES_TO_SIZE(AOS_SHARED_STACK),
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

    params->memory.data_base=data_base;
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
 * 准备验证并加载内核。
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

    params->memory.kernel_base=kernel_base;
    params->memory.kernel_entry=entry;
}

/**
 * 获取单个像素所占位宽。
 * 
 * @param info 图形输出模式信息。
 * 
 * @return 位宽大小，不支持则返回最大值。
 */
STATIC UINT8 EFIAPI boot_get_pixel_bit_width(IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info)
{
    EFI_PIXEL_BITMASK* mask;
    mask=&info->PixelInformation;
    UINT8 width=0;
    UINT32 mask_value=0;
    switch(info->PixelFormat)
    {
        case PixelRedGreenBlueReserved8BitPerColor:
        case PixelBlueGreenRedReserved8BitPerColor:
            return 32;
        case PixelBitMask:
            mask_value=mask->BlueMask|mask->GreenMask|mask->RedMask|mask->ReservedMask;
            if(mask_value==0)
            {
                width=0;
            }
            else
            {
                mask_value>>=(UINT8)LowBitSet32(mask_value);
                width=0;
                while(mask_value&BIT0)
                {
                    width++;
                    mask_value>>=1;
                }
            }
            return width;
        case PixelBltOnly:
        default:
            /*意外情况*/
            return MAX_UINT8;
    }
}

/**
 * EDID结构，下面仅对必要条目进行注释，详细解释看VESA文档。
 */
#pragma pack(1)
typedef struct _boot_edid
{
    /*头部*/
    UINT8  header[8];

    /*厂家和生产厂商ID信息*/
    UINT16 id_manufacturer_name;
    UINT16 id_product_code;
    UINT32 id_serial_number;
    UINT8  week_of_manufacture;
    UINT8  year_of_manufacture;

    /*EDID结构版本和修订号*/
    UINT8  version_number;
    UINT8  revision_number;

    /*基本显示参数和功能*/
    UINT8  video_input_definition;
    UINT8  horizontal_screen_size;
    UINT8  vertical_screen_size;
    UINT8  gamma;
    UINT8  feature_support;

    /*颜色*/
    UINT8  red_green_low;
    UINT8  blue_white_low;
    UINT8  red_x_high;
    UINT8  red_y_high;
    UINT8  green_x_high;
    UINT8  green_y_high;
    UINT8  blue_x_high;
    UINT8  blue_y_high;
    UINT8  white_x_high;
    UINT8  white_y_high;

    /*既定时序*/
    UINT8  established_timings_1;
    UINT8  established_timings_2;
    UINT8  manufacturer_reserved_timings;

    /*标准时序*/
    UINT16 standard_timings[8];

    /*18字节数据块，仅在像素时钟有效情况下下面才有效*/
    struct {
        UINT16 pixel_clock;
        UINT8  horizontal_active_low;
        UINT8  horizontal_blanking_low;
        UINT8  horizontal_high;
        UINT8  vertical_active_low;
        UINT8  vertical_blanking_low;
        UINT8  vertical_high;
        UINT8  horizontal_sync_offset_low;
        UINT8  horizontal_sync_pulse_width_low;
        UINT8  vertical_sync_low;
        UINT8  sync_high;
        UINT8  horizontal_image_size_low;
        UINT8  vertical_image_size_low;
        UINT8  image_size_high;
        UINT8  horizontal_border;
        UINT8  vertical_border;
        UINT8  flags;
    } detailed_timings[4];

    /*扩展块计数*/
    UINT8  extension_block_count;

    /*校验码*/
    UINT8  checksum;
} boot_edid;
#pragma pack()

/**
 * 获取EDID内含信息。
 * 
 * @param gop              含GOP的设备句柄。
 * @param hres             水平分辨率。
 * @param vres             竖直分辨率。
 * @param pixel_clock      像素时钟。
 * @param horizontal_total 水平总行数。
 * @param vertical_total   竖直总列数。
 * @param physical_width   物理屏幕宽度。
 * @param physical_height  物理屏幕高度。
 * 
 * @return 如果EDID正常找到返回真。
 */
STATIC BOOLEAN EFIAPI boot_get_edid_info(IN EFI_HANDLE gop,OUT UINT32* hres,OUT UINT32* vres,OUT UINT32* pixel_clock,
    OUT UINT32* horizontal_total,OUT UINT32* vertical_total,OUT UINT32* physical_width,OUT UINT32* physical_height)
{
    EFI_STATUS status;
    /*协议分两种，第一活动，第二发现，不考虑其他渠道*/
    EFI_EDID_ACTIVE_PROTOCOL* edid_active;
    EFI_EDID_DISCOVERED_PROTOCOL* edid_discovered;
    boot_edid* edid;

    status=gBS->OpenProtocol(gop,&gEfiEdidActiveProtocolGuid,(VOID**)&edid_active,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(status)||edid_active->SizeOfEdid<sizeof(boot_edid))
    {
        /*不支持或者EDID大小为0*/
        status=gBS->OpenProtocol(gop,&gEfiEdidDiscoveredProtocolGuid,(VOID**)&edid_discovered,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if(EFI_ERROR(status)||edid_discovered->SizeOfEdid<sizeof(boot_edid))
        {
            return FALSE;
        }
        else
        {
            edid=(boot_edid*)edid_discovered->Edid;
        }
    }
    else
    {
        edid=(boot_edid*)edid_active->Edid;
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
        return FALSE;
    }
    
    *hres=edid->detailed_timings[0].horizontal_active_low|
        (((UINT32)(edid->detailed_timings[0].horizontal_high&0xF0))<<4);
    *vres=edid->detailed_timings[0].vertical_active_low|
        (((UINT32)(edid->detailed_timings[0].vertical_high&0xF0))<<4);

    /*像素时钟:EDID单位为10kHz，乘10换算为kHz*/
    *pixel_clock=(UINT32)edid->detailed_timings[0].pixel_clock*10;

    /*水平/竖直总量=有效+消隐，消隐高4位分别位于第4、7字节的低4位*/
    *horizontal_total=*hres+(edid->detailed_timings[0].horizontal_blanking_low|
        (((UINT32)(edid->detailed_timings[0].horizontal_high&0x0F))<<8));
    *vertical_total=*vres+(edid->detailed_timings[0].vertical_blanking_low|
        (((UINT32)(edid->detailed_timings[0].vertical_high&0x0F))<<8));

    /*物理尺寸:优先取首选时序的图像尺寸(毫米),无效时回退基本显示参数(厘米)*/
    UINT32 physical_w=edid->detailed_timings[0].horizontal_image_size_low|
        (((UINT32)(edid->detailed_timings[0].image_size_high&0xF0))<<4);
    UINT32 physical_h=edid->detailed_timings[0].vertical_image_size_low|
        (((UINT32)(edid->detailed_timings[0].image_size_high&0x0F))<<8);
    if(physical_w==0)
    {
        physical_w=(UINT32)edid->horizontal_screen_size*10;
    }
    if(physical_h==0)
    {
        physical_h=(UINT32)edid->vertical_screen_size*10;
    }
    *physical_width=physical_w;
    *physical_height=physical_h;
    return TRUE;
}

/**
 * 获取图形输出信息。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_get_graphics_info(IN OUT aos_boot_params* params)
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
    if(status==EFI_BUFFER_TOO_SMALL&&count>0)
    {
        gops=(EFI_HANDLE*)pool_alloc(count);
        if(gops==NULL)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the Graphics Output Protocol array.");
        }

        status=gBS->LocateHandle(ByProtocol,&gEfiGraphicsOutputProtocolGuid,NULL,&count,gops);
        if(EFI_ERROR(status))
        {
            panic(status,"Failed to get the Graphics Output Protocol array.");
        }

        count/=sizeof(EFI_HANDLE);
    }
    else
    {
        panic(status,"Failed to locate the Graphics Output Protocol.");
    }

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop=NULL;
    EFI_HANDLE target;
    for(UINTN index=0;index<count;index++)
    {
        EFI_GRAPHICS_OUTPUT_PROTOCOL* current=NULL;
        status=gBS->OpenProtocol(gops[index],&gEfiGraphicsOutputProtocolGuid,(VOID**)&current,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if(EFI_ERROR(status)||current==NULL)
        {
            continue;
        }
        
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

    pool_free(gops);
    if(gop==NULL)
    {
        panic(EFI_UNSUPPORTED,"The supported Graphics Output Protocol is not found.");
    }

    EFI_DEVICE_PATH_PROTOCOL* device=NULL;
    status=gBS->OpenProtocol(target,&gEfiDevicePathProtocolGuid,(VOID**)&device,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(status))
    {
        panic(EFI_UNSUPPORTED,"The Device Path Protocol is not found within the handle corresponding to"
            " the Graphics Output Protocol.");
    }
    else
    {
        UINTN size=GetDevicePathSize(device);
        if(size==0)
        {
            panic(EFI_DEVICE_ERROR,"The Device Path Protocol is null or invalid.");
        }

        VOID* buffer=pool_alloc(size);
        if(buffer==NULL)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the Device Path Protocol space.");
        }

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
    UINT32 hlim=MAX_UINT32,vlim=MAX_UINT32,clock=0,htotal=0,vtotal=0,pwidth=0,pheight=0;
    BOOLEAN edid=boot_get_edid_info(target,&hlim,&vlim,&clock,&htotal,
        &vtotal,&pwidth,&pheight);
    params->graphics.pixel_clock=clock;
    params->graphics.horizontal_total=htotal;
    params->graphics.vertical_total=vtotal;
    params->graphics.physical_width=pwidth;
    params->graphics.physical_height=pheight;

    if(!edid)
    {
        if(gop->Mode->Info->HorizontalResolution>BOOT_SVGA_HORIZONTAL_RESOLUTION&&
            gop->Mode->Info->VerticalResolution>BOOT_SVGA_VERTICAL_RESOLUTION)
        {
            hlim=gop->Mode->Info->HorizontalResolution;
            vlim=gop->Mode->Info->VerticalResolution;
        }
        else
        {
            UINTN size;
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info=NULL;
            status=gop->QueryMode(gop,0,&size,&info);
            if(status==EFI_SUCCESS)
            {
                if(!((info->HorizontalResolution==BOOT_VGA_HORIZONTAL_RESOLUTION&&
                    info->VerticalResolution==BOOT_VGA_VERTICAL_RESOLUTION)||
                    (info->HorizontalResolution==BOOT_SVGA_HORIZONTAL_RESOLUTION&&
                    info->VerticalResolution==BOOT_SVGA_VERTICAL_RESOLUTION)))
                {
                    hlim=info->HorizontalResolution;
                    vlim=info->VerticalResolution;
                }
                gBS->FreePool(info);
            }
            else
            {
                panic(status,"Failed to get the output mode 0 information.");
            }
        }
    }

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
            continue;
        }
        if(info->HorizontalResolution>hlim||info->VerticalResolution>vlim||info->PixelFormat==PixelBltOnly)
        {
            gBS->FreePool(info);
            continue;
        }
        UINT8 d=boot_get_pixel_bit_width(info);
        if(d==MAX_UINT8)
        {
            gBS->FreePool(info);
            continue;
        }

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
    }

    /*设置模式与输出信息*/
    if(mode!=gop->Mode->Mode)
    {
        status=gop->SetMode(gop,mode);
    }

    params->graphics.hres=gop->Mode->Info->HorizontalResolution;
    params->graphics.vres=gop->Mode->Info->VerticalResolution;
    params->graphics.scan_line=gop->Mode->Info->PixelsPerScanLine;
    params->graphics.framebuffer_base=gop->Mode->FrameBufferBase;
    params->graphics.framebuffer_size=gop->Mode->FrameBufferSize;

    if(!(gop->Mode->Info->PixelFormat==PixelBitMask||
        gop->Mode->Info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor||
        gop->Mode->Info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor))
    {
        panic(EFI_UNSUPPORTED,"Failed to get the color information.");
    }

    if(gop->Mode->Info->PixelFormat==PixelBitMask)
    {
        params->graphics.red_mask=gop->Mode->Info->PixelInformation.RedMask;
        params->graphics.green_mask=gop->Mode->Info->PixelInformation.GreenMask;
        params->graphics.blue_mask=gop->Mode->Info->PixelInformation.BlueMask;
        params->graphics.reserved_mask=gop->Mode->Info->PixelInformation.ReservedMask;
    }
    else if(gop->Mode->Info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor)
    {
        params->graphics.red_mask=0xFF0000;
        params->graphics.green_mask=0xFF00;
        params->graphics.blue_mask=0xFF;
        params->graphics.reserved_mask=0xFF000000;
    }
    else
    {
        params->graphics.red_mask=0xFF;
        params->graphics.green_mask=0xFF00;
        params->graphics.blue_mask=0xFF0000;
        params->graphics.reserved_mask=0xFF000000;
    }
}

/**
 * 设置BSP功能。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_bsp_set(IN OUT aos_boot_params* params)
{
    DisableInterrupts();
    AsmDisableCache();

    IA32_CR0 cr0={.UintN=AsmReadCr0()};
    cr0.Bits.NW=0;
    cr0.Bits.TS=0;
    cr0.Bits.EM=0;
    cr0.Bits.AM=1;
    cr0.Bits.WP=1;
    cr0.Bits.NE=1;
    cr0.Bits.MP=1;
    AsmWriteCr0(cr0.UintN);

    IA32_CR4 cr4={.UintN=AsmReadCr4()};

    if(cr4.Bits.LA57)
    {
        params->state.state|=AOS_STATE_LA57;
    }

    cr4.Bits.OSFXSR=1;
    cr4.Bits.OSXMMEXCPT=1;
    cr4.Bits.PGE=0;

    if(params->features.features&AOS_FEATURES_MC)
    {
        params->state.state|=AOS_STATE_MCE;
        cr4.Bits.MCE=1;
    }
    if(params->features.features&AOS_FEATURES_SMEP)
    {
        params->state.state|=AOS_STATE_SMEP;
        cr4.Bits.SMEP=1;
    }
    if(params->features.features&AOS_FEATURES_SMAP)
    {
        params->state.state|=AOS_STATE_SMAP;
        cr4.Bits.SMAP=1;
    }
    
    AsmWriteCr4(cr4.UintN);
    CpuFlushTlb();

    MSR_IA32_MTRR_DEF_TYPE_REGISTER mtrrdef={.Uint64=AsmReadMsr64(MSR_IA32_MTRR_DEF_TYPE)};
    mtrrdef.Bits.E=0;
    AsmWriteMsr64(MSR_IA32_MTRR_DEF_TYPE,mtrrdef.Uint64);

    if(params->state.state&AOS_STATE_FIXED_MTRR)
    {
        mtrrdef.Bits.FE=1;
        /**
         * 环境使用的固定MTRR内存类型。
         */
        #define BOOT_FIXED_MTRR_TYPE ((UINT64)SIGNATURE_64(MSR_IA32_MTRR_CACHE_WRITE_BACK,\
            MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,\
            MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,MSR_IA32_MTRR_CACHE_WRITE_BACK,\
            MSR_IA32_MTRR_CACHE_WRITE_BACK))
        AsmWriteMsr64(MSR_IA32_MTRR_FIX64K_00000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX16K_80000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX16K_A0000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_C0000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_C8000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_D0000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_D8000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_E0000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_E8000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_F0000,BOOT_FIXED_MTRR_TYPE);
        AsmWriteMsr64(MSR_IA32_MTRR_FIX4K_F8000,BOOT_FIXED_MTRR_TYPE);
        #undef BOOT_FIXED_MTRR_TYPE
    }

    for(UINT8 index=0;index<params->state.vmtrr;index++)
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
    if(params->features.features&AOS_FEATURES_NX)
    {
        efer.Bits.NXE=1;
    }
    efer.Bits.SCE=1;
    AsmWriteMsr64(MSR_IA32_EFER,efer.Uint64);

    mtrrdef.Bits.E=1;
    mtrrdef.Bits.Type=MSR_IA32_MTRR_CACHE_WRITE_BACK;
    AsmWriteMsr64(MSR_IA32_MTRR_DEF_TYPE,mtrrdef.Uint64);

    CpuFlushTlb();
    AsmEnableCache();
    cr4.Bits.PGE=1;
    AsmWriteCr4(cr4.UintN);

    if(params->features.vendor==AOS_VENDOR_INTEL)
    {
        MSR_IA32_MISC_ENABLE_REGISTER misc={.Uint64=AsmReadMsr64(MSR_IA32_MISC_ENABLE)};
        misc.Bits.FastStrings=1;
        AsmWriteMsr64(MSR_IA32_MISC_ENABLE,misc.Uint64);
    }
}

/**
 * 转成内核页表。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_use_kernel_page_table(IN aos_boot_params* params)
{
    IA32_CR4 cr4={.UintN=AsmReadCr4()};
    cr4.Bits.PGE=0;
    AsmWriteCr4(cr4.UintN);

    AsmWriteCr3(params->memory.page_table);

    cr4.Bits.PGE=1;
    AsmWriteCr4(cr4.UintN);

    IA32_DESCRIPTOR gdtr;
    gdtr.Base=data_base+kernel_gdt;
    gdtr.Limit=sizeof(AOS_FIXED_GDT)+sizeof(IA32_TSS_DESCRIPTOR)-1;
    AsmWriteGdtr(&gdtr);

    set_cs(24);
    set_ds(32);
    AsmWriteTr(64);

    gdtr.Base=share_base;
    AsmWriteGdtr(&gdtr);

    IA32_DESCRIPTOR idtr;
    idtr.Base=share_base+SIZE_8KB;
    idtr.Limit=sizeof(IA32_IDT_GATE_DESCRIPTOR)*256-1;
    AsmWriteIdtr(&idtr);
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
    ZeroMem(params,sizeof(aos_boot_params));

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

    /*准备CPU数据*/
    boot_prepare_cpu_data(params);

    /*验证并载入内核*/
    boot_verify_and_load(params);

    /*获取图形信息*/
    boot_get_graphics_info(params);

    /*准备处理映射表*/
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

    params->memory.kernel_vma=fill_firmware_vma(firmware_base,memmap,map_size,desc_size);
    params->memory.memory_map=fill_memory_map(memmap,map_size,desc_size,bpbase);

    UINTN free_pages=0;
    aos_boot_memory* node=params->memory.memory_map;
    while(node!=NULL)
    {
        if(node->type==AOS_BOOT_MEMORY_FREE)
        {
            free_pages+=EFI_SIZE_TO_PAGES(node->size);
        }
        node=node->next;
    }

    if(free_pages<CONFIG_MINIMUM_FREE_PAGES)
    {
        panic(EFI_OUT_OF_RESOURCES,"The system is low on free memory.");
    }

    status=gBS->ExitBootServices(gImageHandle,map_key);
    if(EFI_ERROR(status))
    {
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }

    boot_bsp_set(params);
    boot_use_kernel_page_table(params);

    status=gRT->SetVirtualAddressMap(map_size,desc_size,desc_ver,memmap);
    if(EFI_ERROR(status))
    {
        gRT->ResetSystem(EfiResetShutdown,status,0,NULL);
        return status;
    }
    params->system_table=(aos_efi_system_table*)(firmware_base+(UINTN)system_table);

    /*令其他AP初始化*/
    if(params->state.apic==AOS_APIC_XAPIC)
    {
        MSR_IA32_APIC_BASE_REGISTER apic={.Uint64=AsmReadMsr64(MSR_IA32_APIC_BASE)};
        UINTN base=((UINTN)apic.Bits.ApicBase<<12)|((UINTN)apic.Bits.ApicBaseHi<<32);
        MmioWrite32(base+0x310,0);
        MmioWrite32(base+0x300,0xC4500);
    }
    else if(params->state.apic==AOS_APIC_X2APIC)
    {
        AsmWriteMsr64(0x830,0xC4500);
    }

    /*进入内核*/
    aos_kernel_trampoline trampoline=(aos_kernel_trampoline)(kernel_base+params->memory.kernel_entry);
    trampoline(params,stack_base+EFI_PAGES_TO_SIZE(CONFIG_KERNEL_STACK));
    
    UNREACHABLE();
    gRT->ResetSystem(EfiResetShutdown,EFI_DEVICE_ERROR,0,NULL);
    return EFI_DEVICE_ERROR;
}
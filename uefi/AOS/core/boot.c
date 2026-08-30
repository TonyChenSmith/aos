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
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Register/Amd/ArchitecturalMsr.h>
#include <Register/Amd/Cpuid.h>
#include <Register/Intel/ArchitecturalMsr.h>
#include <Register/Intel/Cpuid.h>
#include <Register/Intel/LocalApic.h>

#include "config.h"
#include "mark.h"
#include "panic.h"
#include "params.h"
#include "pool.h"

/**
 * 环境使用的GDT设置。
 */
CONST UINT64 ENV_GDT[]={
    0x0000000000000000ULL, /*NULL*/
    0x00CF9A000000FFFFULL, /*KERNEL_CODE32*/
    0x00CF92000000FFFFULL, /*KERNEL_DATA32*/
    0x00AF9A000000FFFFULL, /*KERNEL_CODE64*/
    0x00AF92000000FFFFULL, /*KERNEL_DATA64*/
    0x00AFFA000000FFFFULL, /*USER_CODE64*/
    0x00AFF2000000FFFFULL, /*USER_DATA64*/
    0x0000000000000000ULL, /*BOUNDARY*/
};

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
 * 申请并准备内核所需数据结构。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI boot_prepare_data(IN OUT aos_boot_params* params)
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
        UINT64 base=(apic_base.Bits.ApicBase<<12)|((UINT64)apic_base.Bits.ApicBaseHi<<32);
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
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate AOS boot parameters from bootstrap pool.");
    }

    /*CPU检查*/
    boot_cpu_check(params);

    /*设置系统表*/
    boot_set_table(params);

    /*准备启动数据*/
    boot_prepare_data(params);

    panic_if_error(EFI_DEVICE_ERROR,"Hello?");
    return EFI_SUCCESS;
}
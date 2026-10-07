/**
 * 模块内核加载。
 * @date 2026-10-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Library/BaseMemoryLib.h>
#include "file.h"
#include "mark.h"
#include "panic.h"
#include "pool.h"
#include "vma.h"

/**
 * ELF 64位数据类型。
 */
typedef UINT64 load_elf64_addr;
typedef UINT16 load_elf64_half;
typedef UINT64 load_elf64_off;
typedef INT32 load_elf64_sword;
typedef UINT32 load_elf64_word;
typedef UINT64 load_elf64_xword;
typedef INT64 load_elf64_sxword;
typedef UINT8 load_elf64_uchar;

/**
 * ELF头。
 */
typedef struct _load_elf64_ehdr
{
    load_elf64_uchar e_ident[16]; /*标识。*/
    load_elf64_half  e_type;      /*类型。*/
    load_elf64_half  e_machine;   /*机器。*/
    load_elf64_word  e_version;   /*版本。*/
    load_elf64_addr  e_entry;     /*入口地址。*/
    load_elf64_off   e_phoff;     /*程序头偏移。*/
    load_elf64_off   e_shoff;     /*节头偏移。*/
    load_elf64_word  e_flags;     /*处理器特定标志。*/
    load_elf64_half  e_ehsize;    /*ELF头大小。*/
    load_elf64_half  e_phentsize; /*程序头条目大小。*/
    load_elf64_half  e_phnum;     /*程序头条目数目。*/
    load_elf64_half  e_shentsize; /*节头条目大小。*/
    load_elf64_half  e_shnum;     /*节头条目数目。*/
    load_elf64_half  e_shstrndx;  /*节字符串表索引。*/
} load_elf64_ehdr;

/**
 * 程序头。
 */
typedef struct _load_elf64_phdr
{
    load_elf64_word  p_type;   /*类型。*/
    load_elf64_word  p_flags;  /*标志。*/
    load_elf64_off   p_offset; /*偏移。*/
    load_elf64_addr  p_vaddr;  /*虚拟基址。*/
    load_elf64_addr  p_paddr;  /*物理基址。*/
    load_elf64_xword p_filesz; /*文件内大小。*/
    load_elf64_xword p_memsz;  /*内存内大小。*/
    load_elf64_xword p_align;  /*对齐。*/
} load_elf64_phdr;

/**
 * 动态表。
 */
typedef struct _load_elf64_dyn
{
    load_elf64_xword d_tag;     /*标签。*/
    union
    {
        load_elf64_xword d_val; /*值。*/
        load_elf64_addr d_ptr;  /*指针。*/
    } d_un;
} load_elf64_dyn;

/**
 * ELF标识索引。
 */
#define LOAD_EI_MAG0 0
#define LOAD_EI_MAG1 1
#define LOAD_EI_MAG2 2
#define LOAD_EI_MAG3 3
#define LOAD_EI_CLASS 4
#define LOAD_EI_DATA 5
#define LOAD_EI_VERSION 6
#define LOAD_EI_OSABI 7
#define LOAD_EI_ABIVERSION 8
#define LOAD_EI_PAD 9
#define LOAD_EI_NIDENT 16

/**
 * ELF标识。
 */
#define LOAD_ELFMAG0 0x7F
#define LOAD_ELFMAG1 'E'
#define LOAD_ELFMAG2 'L'
#define LOAD_ELFMAG3 'F'

#define LOAD_ELFCLASSNONE 0
#define LOAD_ELFCLASS32 1
#define LOAD_ELFCLASS64 2

#define LOAD_ELFDATANONE 0
#define LOAD_ELFDATA2LSB 1
#define LOAD_ELFDATA2MSB 2

#define LOAD_ELFOSABI_NONE 0
#define LOAD_ELFOSABI_SYSV 0

/**
 * ELF类型。
 */
#define LOAD_ET_NONE 0
#define LOAD_ET_REL 1
#define LOAD_ET_EXEC 2
#define LOAD_ET_DYN 3
#define LOAD_ET_CORE 4
#define LOAD_ET_LOPROC 0xFF00
#define LOAD_ET_HIPROC 0xFFFF

/**
 * ELF机器。
 */
#define LOAD_EM_NONE 0
#define LOAD_EM_386 3
#define LOAD_EM_AMD64 62

/**
 * ELF版本。
 */
#define LOAD_EV_NONE 0
#define LOAD_EV_CURRENT 1

/**
 * 程序头类型。
 */
#define LOAD_PT_NULL 0
#define LOAD_PT_LOAD 1
#define LOAD_PT_DYNAMIC 2
#define LOAD_PT_INTERP 3
#define LOAD_PT_NOTE 4
#define LOAD_PT_SHLIB 5
#define LOAD_PT_PHDR 6
#define LOAD_PT_TLS 7
#define LOAD_PT_LOOS 0x60000000
#define LOAD_PT_HIOS 0x6FFFFFFF
#define LOAD_PT_LOPROC 0x70000000
#define LOAD_PT_HIPROC 0x7FFFFFFF

/**
 * 程序头标志。
 */
#define LOAD_PF_X 0x1
#define LOAD_PF_W 0x2
#define LOAD_PF_R 0x4
#define LOAD_PF_MASKPROC 0xF0000000

/**
 * 动态表标签。
 */
#define LOAD_DT_NULL 0
#define LOAD_DT_NEEDED 1
#define LOAD_DT_PLTRELSZ 2
#define LOAD_DT_PLTGOT 3
#define LOAD_DT_HASH 4
#define LOAD_DT_STRTAB 5
#define LOAD_DT_SYMTAB 6
#define LOAD_DT_RELA 7
#define LOAD_DT_RELASZ 8
#define LOAD_DT_RELAENT 9
#define LOAD_DT_STRSZ 10
#define LOAD_DT_SYMENT 11
#define LOAD_DT_INIT 12
#define LOAD_DT_FINI 13
#define LOAD_DT_SONAME 14
#define LOAD_DT_RPATH 15
#define LOAD_DT_SYMBOLIC 16
#define LOAD_DT_REL 17
#define LOAD_DT_RELSZ 18
#define LOAD_DT_RELENT 19
#define LOAD_DT_PLTREL 20
#define LOAD_DT_DEBUG 21
#define LOAD_DT_TEXTREL 22
#define LOAD_DT_JMPREL 23
#define LOAD_DT_BIND_NOW 24
#define LOAD_DT_INIT_ARRAY 25
#define LOAD_DT_FINI_ARRAY 26
#define LOAD_DT_INIT_ARRAYSZ 27
#define LOAD_DT_FINI_ARRAYSZ 28
#define LOAD_DT_RUNPATH 29
#define LOAD_DT_FLAGS 30
#define LOAD_DT_ENCODING 32
#define LOAD_DT_PREINIT_ARRAY 32
#define LOAD_DT_PREINIT_ARRAYSZ 33
#define LOAD_DT_SYMTAB_SHNDX 34
#define LOAD_DT_LOOS 0x6000000D
#define LOAD_DT_HIOS 0x6FFFF000
#define LOAD_DT_VALRNGLO 0x6FFFFD00
#define LOAD_DT_CHECKSUM 0x6FFFFDF8
#define LOAD_DT_PLTPADSZ 0x6FFFFDF9
#define LOAD_DT_MOVEENT 0x6FFFFDFA
#define LOAD_DT_MOVESZ 0x6FFFFDFB
#define LOAD_DT_POSFLAG_1 0x6FFFFDFD
#define LOAD_DT_SYMINSZ 0x6FFFFDFE
#define LOAD_DT_SYMINENT 0x6FFFFDFF
#define LOAD_DT_VALRNGHI 0x6FFFFDFF
#define LOAD_DT_ADDRRNGLO 0x6FFFFE00
#define LOAD_DT_CONFIG 0x6FFFFEFA
#define LOAD_DT_DEPAUDIT 0x6FFFFEFB
#define LOAD_DT_AUDIT 0x6FFFFEFC
#define LOAD_DT_PLTPAD 0x6FFFFEFD
#define LOAD_DT_MOVETAB 0x6FFFFEFE
#define LOAD_DT_SYMINFO 0x6FFFFEFF
#define LOAD_DT_ADDRRNGHI 0x6FFFFEFF
#define LOAD_DT_VERSYM 0x6FFFFFF0
#define LOAD_DT_RELACOUNT 0x6FFFFFF9
#define LOAD_DT_RELCOUNT 0x6FFFFFFA
#define LOAD_DT_FLAGS_1 0x6FFFFFFB
#define LOAD_DT_VERDEF 0x6FFFFFFC
#define LOAD_DT_VERDEFNUM 0x6FFFFFFD
#define LOAD_DT_VERNEED 0x6FFFFFFE
#define LOAD_DT_VERNEEDNUM 0x6FFFFFFF
#define LOAD_DT_LOPROC 0x70000000
#define LOAD_DT_SPARC_REGISTER 0x70000001
#define LOAD_DT_AUXILIARY 0x7FFFFFFD
#define LOAD_DT_USED 0x7FFFFFFE
#define LOAD_DT_FILTER 0x7FFFFFFF
#define LOAD_DT_HIPROC 0x7FFFFFFF

/**
 * 动态表FLAG标签标志。
 */
#define LOAD_DF_ORIGIN 0x1
#define LOAD_DF_SYMBOLIC 0x2
#define LOAD_DF_TEXTREL 0x4
#define LOAD_DF_BIND_NOW 0x8
#define LOAD_DF_STATIC_TLS 0x10

/**
 * 动态表FLAG_1标签标志。
 */
#define LOAD_DF_1_NOW 0x1
#define LOAD_DF_1_GLOBAL 0x2
#define LOAD_DF_1_GROUP 0x4
#define LOAD_DF_1_NODELETE 0x8
#define LOAD_DF_1_LOADFLTR 0x10
#define LOAD_DF_1_INITFIRST 0x20
#define LOAD_DF_1_NOOPEN 0x40
#define LOAD_DF_1_ORIGIN 0x80
#define LOAD_DF_1_DIRECT 0x100
#define LOAD_DF_1_INTERPOSE 0x400
#define LOAD_DF_1_NODEFLIB 0x800
#define LOAD_DF_1_NODUMP 0x1000
#define LOAD_DF_1_ENDFILTEE 0x4000
#define LOAD_DF_1_DISPRELDNE 0x8000
#define LOAD_DF_1_DISPRELPND 0x10000
#define LOAD_DF_1_NODIRECT 0x20000
#define LOAD_DF_1_IGNMULDEF 0x40000
#define LOAD_DF_1_NOKSYMS 0x80000
#define LOAD_DF_1_NOHDR 0x100000
#define LOAD_DF_1_EDITED 0x200000
#define LOAD_DF_1_NORELOC 0x400000
#define LOAD_DF_1_SYMINTPOSE 0x800000
#define LOAD_DF_1_GLOBAUDIT 0x1000000
#define LOAD_DF_1_SINGLETON 0x2000000
#define LOAD_DF_1_STUB 0x4000000
#define LOAD_DF_1_PIE 0x8000000
#define LOAD_DF_1_KMOD 0x10000000
#define LOAD_DF_1_WEAKFILTER 0x20000000

/**
 * 动态表POSFLAG_1标签标志。
 */
#define LOAD_DF_P1_LAZYLOAD 0x1
#define LOAD_DF_P1_GROUPPERM 0x2
#define LOAD_DF_P1_DEFERRED 0x4
#define LOAD_DF_P1_EXISTING 0x8

/**
 * 内核可加载段线性区域大小上限。
 */
#define LOAD_KERNEL_MAX_SIZE SIZE_2GB

/**
 * 对内核的ELF头进行检查。
 * 
 * @param ehdr ELF头。
 * @param size 内核文件大小。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI load_elf_check_header(IN load_elf64_ehdr* ehdr,IN UINT64 size)
{
    if(ehdr->e_ident[LOAD_EI_MAG0]!=LOAD_ELFMAG0||ehdr->e_ident[LOAD_EI_MAG1]!=LOAD_ELFMAG1||
        ehdr->e_ident[LOAD_EI_MAG2]!=LOAD_ELFMAG2||ehdr->e_ident[LOAD_EI_MAG3]!=LOAD_ELFMAG3||
        ehdr->e_ident[LOAD_EI_CLASS]!=LOAD_ELFCLASS64||ehdr->e_ident[LOAD_EI_DATA]!=LOAD_ELFDATA2LSB||
        ehdr->e_ident[LOAD_EI_VERSION]!=LOAD_EV_CURRENT||ehdr->e_ident[LOAD_EI_OSABI]!=LOAD_ELFOSABI_SYSV)
    {
        panic(EFI_UNSUPPORTED,"The kernel file is not a supported ELF file.");
    }
    if(ehdr->e_type!=LOAD_ET_DYN||ehdr->e_machine!=LOAD_EM_AMD64||ehdr->e_version!=LOAD_EV_CURRENT||
        ehdr->e_ehsize!=sizeof(load_elf64_ehdr)||ehdr->e_phentsize!=sizeof(load_elf64_phdr))
    {
        panic(EFI_UNSUPPORTED,"The kernel file's ELF type is not supported.");
    }
    UINT64 phdr_bytes=(UINT64)ehdr->e_phnum*sizeof(load_elf64_phdr);
    if(ehdr->e_phnum==0||ehdr->e_phoff>size||phdr_bytes>size-ehdr->e_phoff)
    {
        panic(EFI_UNSUPPORTED,"The kernel file's program header table is out of range.");
    }
}

/**
 * 对内核的ELF程序头进行检查。
 * 
 * @param phdrs ELF程序头数组。
 * @param phnum ELF程序头数目。
 * @param size  内核文件大小。
 * @param entry 内核程序入口偏移。
 * 
 * @return 所需线性区域大小。
 */
STATIC UINT64 EFIAPI load_elf_check_phdrs(IN load_elf64_phdr* phdrs,IN UINT16 phnum,IN UINT64 size,IN UINT64 entry)
{
    UINT64 load_count=0,max_vaddr=0;
    BOOLEAN entry_covered=FALSE;
    for(UINT16 i=0;i<phnum;i++)
    {
        if(phdrs[i].p_type!=LOAD_PT_LOAD||(phdrs[i].p_filesz==0&&phdrs[i].p_memsz==0))
        {
            continue;
        }
        
        if(phdrs[i].p_filesz>phdrs[i].p_memsz||phdrs[i].p_offset>size||phdrs[i].p_filesz>size-phdrs[i].p_offset)
        {
            panic(EFI_UNSUPPORTED,"The kernel file's segment range is invalid.");
        }
        
        if(phdrs[i].p_vaddr>LOAD_KERNEL_MAX_SIZE-phdrs[i].p_memsz)
        {
            panic(EFI_UNSUPPORTED,"The kernel file's address range is invalid.");
        }

        if(phdrs[i].p_vaddr+phdrs[i].p_memsz>max_vaddr)
        {
            max_vaddr=phdrs[i].p_vaddr+phdrs[i].p_memsz;
        }
        if((phdrs[i].p_flags&LOAD_PF_X)!=0&&entry>=phdrs[i].p_vaddr&&entry<phdrs[i].p_vaddr+phdrs[i].p_memsz)
        {
            entry_covered=TRUE;
        }
        load_count++;
    }
    if(load_count==0)
    {
        panic(EFI_UNSUPPORTED,"The kernel file has no loadable segment.");
    }
    if(!entry_covered)
    {
        panic(EFI_UNSUPPORTED,"The kernel entry is not covered by an executable segment.");
    }
    return max_vaddr;
}

/**
 * 加载并映射内核文件。内核必须已完成签名验证。
 * 
 * @param kernel 内核文件句柄。
 * @param base   内核线性基址。
 * 
 * @return 成功加载后返回入口偏移。
 */
UINTN EFIAPI load_kernel(IN asv_file* kernel,IN UINTN base)
{
    if(kernel==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The kernel file is null.");
    }

    UINT64 size=asv_get_size(kernel);
    if(size<sizeof(load_elf64_ehdr))
    {
        panic(EFI_UNSUPPORTED,"The kernel file is too small.");
    }

    load_elf64_ehdr* ehdr=pool_alloc(sizeof(load_elf64_ehdr));
    if(ehdr==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Not enough memory to load the kernel.");
    }
    if(asv_read_at(kernel,0,ehdr,sizeof(load_elf64_ehdr))!=sizeof(load_elf64_ehdr))
    {
        panic(EFI_DEVICE_ERROR,"Failed to read the kernel file.");
    }
    load_elf_check_header(ehdr,size);

    UINT64 phdr_bytes=(UINT64)ehdr->e_phnum*sizeof(load_elf64_phdr);
    load_elf64_phdr* phdrs=pool_alloc(phdr_bytes);
    if(phdrs==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Not enough memory to load the kernel.");
    }
    if(asv_read_at(kernel,ehdr->e_phoff,phdrs,phdr_bytes)!=phdr_bytes)
    {
        panic(EFI_DEVICE_ERROR,"Failed to read the kernel file.");
    }
    load_elf_check_phdrs(phdrs,ehdr->e_phnum,size,ehdr->e_entry);

    for(UINT16 i=0;i<ehdr->e_phnum;i++)
    {
        if(phdrs[i].p_type!=LOAD_PT_LOAD||(phdrs[i].p_filesz==0&&phdrs[i].p_memsz==0))
        {
            continue;
        }

        UINTN pages=EFI_SIZE_TO_PAGES(phdrs[i].p_vaddr+phdrs[i].p_memsz)-(phdrs[i].p_vaddr>>EFI_PAGE_SHIFT);
        UINTN paddr=(UINTN)allocate_page(AOS_BOOT_MEMORY_KERNEL,pages,SIZE_4GB-1);
        if(paddr==0)
        {
            panic(EFI_OUT_OF_RESOURCES,"Failed to allocate the kernel segment space.");
        }

        UINT64 flags=AOS_BOOT_VMA_READ|AOS_BOOT_VMA_CACHE_WB;
        if(phdrs[i].p_flags&LOAD_PF_W)
        {
            flags|=AOS_BOOT_VMA_WRITE;
        }

        if(phdrs[i].p_flags&LOAD_PF_X)
        {
            flags|=AOS_BOOT_VMA_EXECUTE;
            SetMem((VOID*)paddr,EFI_PAGES_TO_SIZE(pages),0xCC);
        }
        else
        {
            ZeroMem((VOID*)paddr,EFI_PAGES_TO_SIZE(pages));
        }

        if(phdrs[i].p_filesz>0)
        {
            if(asv_read_at(kernel,phdrs[i].p_offset,(VOID*)(paddr+(phdrs[i].p_vaddr&0xFFF)),
                phdrs[i].p_filesz)!=phdrs[i].p_filesz)
            {
                panic(EFI_DEVICE_ERROR,"Failed to read the kernel file.");
            }
        }

        UINTN vaddr=base+phdrs[i].p_vaddr;
        add_vma(AOS_BOOT_VMA_TYPE_KERNEL,(vaddr>>EFI_PAGE_SHIFT)<<EFI_PAGE_SHIFT,paddr,pages,flags);
    }

    UINTN result=ehdr->e_entry;

    pool_free(ehdr);
    pool_free(phdrs);

    return result;
}
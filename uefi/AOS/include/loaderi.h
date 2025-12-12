/**
 * 模块内核程序文件加载所需的不公开内容。
 * @date 2025-12-09
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_LOADER_INTERNAL_H__
#define __AOS_UEFI_LOADER_INTERNAL_H__

#include "loader.h"

/**
 * ELF 64位数据类型。
 */
typedef UINT64 loader_elf64_addr;
typedef UINT16 loader_elf64_half;
typedef UINT64 loader_elf64_off;
typedef INT32 loader_elf64_sword;
typedef UINT32 loader_elf64_word;
typedef UINT64 loader_elf64_xword;
typedef INT64 loader_elf64_sxword;
typedef UINT8 loader_elf64_uchar;

/**
 * ELF头。
 */
typedef struct _loader_elf64_ehdr
{
    loader_elf64_uchar e_ident[16]; /*标识。*/
    loader_elf64_half  e_type;             /*类型。*/
    loader_elf64_half  e_machine;          /*机器。*/
    loader_elf64_word  e_version;          /*版本。*/
    loader_elf64_addr  e_entry;            /*入口地址。*/
    loader_elf64_off   e_phoff;            /*程序头偏移。*/
    loader_elf64_off   e_shoff;            /*节头偏移。*/
    loader_elf64_word  e_flags;            /*处理器特定标志。*/
    loader_elf64_half  e_ehsize;           /*ELF头大小。*/
    loader_elf64_half  e_phentsize;        /*程序头条目大小。*/
    loader_elf64_half  e_phnum;            /*程序头条目数目。*/
    loader_elf64_half  e_shentsize;        /*节头条目大小。*/
    loader_elf64_half  e_shnum;            /*节头条目数目。*/
    loader_elf64_half  e_shstrndx;         /*节字符串表索引。*/
} loader_elf64_ehdr;

/**
 * 程序头。
 */
typedef struct _loader_elf64_phdr
{
    loader_elf64_word  p_type;   /*类型。*/
    loader_elf64_word  p_flags;  /*标志。*/
    loader_elf64_off   p_offset; /*偏移。*/
    loader_elf64_addr  p_vaddr;  /*虚拟基址。*/
    loader_elf64_addr  p_paddr;  /*物理基址。*/
    loader_elf64_xword p_filesz; /*文件内大小。*/
    loader_elf64_xword p_memsz;  /*内存内大小。*/
    loader_elf64_xword p_align;  /*对齐。*/
} loader_elf64_phdr;

/**
 * 动态表。
 */
typedef struct _loader_elf64_dyn
{
    loader_elf64_xword d_tag;     /*标签。*/
    union
    {
        loader_elf64_xword d_val; /*值。*/
        loader_elf64_addr d_ptr;  /*指针。*/
    } d_un;
} elf64_dyn;

/**
 * ELF标识索引。
 */
#define LOADER_EI_MAG0 0
#define LOADER_EI_MAG1 1
#define LOADER_EI_MAG2 2
#define LOADER_EI_MAG3 3
#define LOADER_EI_CLASS 4
#define LOADER_EI_DATA 5
#define LOADER_EI_VERSION 6
#define LOADER_EI_OSABI 7
#define LOADER_EI_ABIVERSION 8
#define LOADER_EI_PAD 9
#define LOADER_EI_NIDENT 16

/**
 * ELF标识。
 */
#define LOADER_ELFMAG0 0x7F
#define LOADER_ELFMAG1 'E'
#define LOADER_ELFMAG2 'L'
#define LOADER_ELFMAG3 'F'

#define LOADER_ELFCLASSNONE 0
#define LOADER_ELFCLASS32 1
#define LOADER_ELFCLASS64 2

#define LOADER_ELFDATANONE 0
#define LOADER_ELFDATA2LSB 1
#define LOADER_ELFDATA2MSB 2

#define LOADER_ELFOSABI_NONE 0
#define LOADER_ELFOSABI_SYSV 0

/**
 * ELF类型。
 */
#define LOADER_ET_NONE 0
#define LOADER_ET_REL 1
#define LOADER_ET_EXEC 2
#define LOADER_ET_DYN 3
#define LOADER_ET_CORE 4
#define LOADER_ET_LOPROC 0xFF00
#define LOADER_ET_HIPROC 0xFFFF

/**
 * ELF机器。
 */
#define LOADER_EM_NONE 0
#define LOADER_EM_386 3
#define LOADER_EM_AMD64 62

/**
 * ELF版本。
 */
#define LOADER_EV_NONE 0
#define LOADER_EV_CURRENT 1

/**
 * 程序头类型。
 */
#define LOADER_PT_NULL 0
#define LOADER_PT_LOAD 1
#define LOADER_PT_DYNAMIC 2
#define LOADER_PT_INTERP 3
#define LOADER_PT_NOTE 4
#define LOADER_PT_SHLIB 5
#define LOADER_PT_PHDR 6
#define LOADER_PT_TLS 7
#define LOADER_PT_LOOS 0x60000000
#define LOADER_PT_HIOS 0x6FFFFFFF
#define LOADER_PT_LOPROC 0x70000000
#define LOADER_PT_HIPROC 0x7FFFFFFF

/**
 * 程序头标志。
 */
#define LOADER_PF_X 0x1
#define LOADER_PF_W 0x2
#define LOADER_PF_R 0x4
#define LOADER_PF_MASKPROC 0xF0000000

/**
 * 动态表标签。
 */
#define LOADER_DT_NULL 0
#define LOADER_DT_NEEDED 1
#define LOADER_DT_PLTRELSZ 2
#define LOADER_DT_PLTGOT 3
#define LOADER_DT_HASH 4
#define LOADER_DT_STRTAB 5
#define LOADER_DT_SYMTAB 6
#define LOADER_DT_RELA 7
#define LOADER_DT_RELASZ 8
#define LOADER_DT_RELAENT 9
#define LOADER_DT_STRSZ 10
#define LOADER_DT_SYMENT 11
#define LOADER_DT_INIT 12
#define LOADER_DT_FINI 13
#define LOADER_DT_SONAME 14
#define LOADER_DT_RPATH 15
#define LOADER_DT_SYMBOLIC 16
#define LOADER_DT_REL 17
#define LOADER_DT_RELSZ 18
#define LOADER_DT_RELENT 19
#define LOADER_DT_PLTREL 20
#define LOADER_DT_DEBUG 21
#define LOADER_DT_TEXTREL 22
#define LOADER_DT_JMPREL 23
#define LOADER_DT_BIND_NOW 24
#define LOADER_DT_INIT_ARRAY 25
#define LOADER_DT_FINI_ARRAY 26
#define LOADER_DT_INIT_ARRAYSZ 27
#define LOADER_DT_FINI_ARRAYSZ 28
#define LOADER_DT_RUNPATH 29
#define LOADER_DT_FLAGS 30
#define LOADER_DT_ENCODING 32
#define LOADER_DT_PREINIT_ARRAY 32
#define LOADER_DT_PREINIT_ARRAYSZ 33
#define LOADER_DT_SYMTAB_SHNDX 34
#define LOADER_DT_LOOS 0x6000000D
#define LOADER_DT_HIOS 0x6FFFF000
#define LOADER_DT_VALRNGLO 0x6FFFFD00
#define LOADER_DT_CHECKSUM 0x6FFFFDF8
#define LOADER_DT_PLTPADSZ 0x6FFFFDF9
#define LOADER_DT_MOVEENT 0x6FFFFDFA
#define LOADER_DT_MOVESZ 0x6FFFFDFB
#define LOADER_DT_POSFLAG_1 0x6FFFFDFD
#define LOADER_DT_SYMINSZ 0x6FFFFDFE
#define LOADER_DT_SYMINENT 0x6FFFFDFF
#define LOADER_DT_VALRNGHI 0x6FFFFDFF
#define LOADER_DT_ADDRRNGLO 0x6FFFFE00
#define LOADER_DT_CONFIG 0x6FFFFEFA
#define LOADER_DT_DEPAUDIT 0x6FFFFEFB
#define LOADER_DT_AUDIT 0x6FFFFEFC
#define LOADER_DT_PLTPAD 0x6FFFFEFD
#define LOADER_DT_MOVETAB 0x6FFFFEFE
#define LOADER_DT_SYMINFO 0x6FFFFEFF
#define LOADER_DT_ADDRRNGHI 0x6FFFFEFF
#define LOADER_DT_VERSYM 0x6FFFFFF0
#define LOADER_DT_RELACOUNT 0x6FFFFFF9
#define LOADER_DT_RELCOUNT 0x6FFFFFFA
#define LOADER_DT_FLAGS_1 0x6FFFFFFB
#define LOADER_DT_VERDEF 0x6FFFFFFC
#define LOADER_DT_VERDEFNUM 0x6FFFFFFD
#define LOADER_DT_VERNEED 0x6FFFFFFE
#define LOADER_DT_VERNEEDNUM 0x6FFFFFFF
#define LOADER_DT_LOPROC 0x70000000
#define LOADER_DT_SPARC_REGISTER 0x70000001
#define LOADER_DT_AUXILIARY 0x7FFFFFFD
#define LOADER_DT_USED 0x7FFFFFFE
#define LOADER_DT_FILTER 0X7FFFFFFF
#define LOADER_DT_HIPROC 0x7FFFFFFF

/**
 * 动态表FLAG标签标志。
 */
#define LOADER_DF_ORIGIN 0x1
#define LOADER_DF_SYMBOLIC 0x2
#define LOADER_DF_TEXTREL 0x4
#define LOADER_DF_BIND_NOW 0x8
#define LOADER_DF_STATIC_TLS 0x10

/**
 * 动态表FLAG_1标签标志。
 */
#define LOADER_DF_1_NOW 0x1
#define LOADER_DF_1_GLOBAL 0x2
#define LOADER_DF_1_GROUP 0x4
#define LOADER_DF_1_NODELETE 0x8
#define LOADER_DF_1_LOADFLTR 0x10
#define LOADER_DF_1_INITFIRST 0x20
#define LOADER_DF_1_NOOPEN 0x40
#define LOADER_DF_1_ORIGIN 0x80
#define LOADER_DF_1_DIRECT 0x100
#define LOADER_DF_1_INTERPOSE 0x400
#define LOADER_DF_1_NODEFLIB 0x800
#define LOADER_DF_1_NODUMP 0x1000
#define LOADER_DF_1_ENDFILTEE 0x4000
#define LOADER_DF_1_DISPRELDNE 0x8000
#define LOADER_DF_1_DISPRELPND 0x10000
#define LOADER_DF_1_NODIRECT 0x20000
#define LOADER_DF_1_IGNMULDEF 0x40000
#define LOADER_DF_1_NOKSYMS 0x80000
#define LOADER_DF_1_NOHDR 0x100000
#define LOADER_DF_1_EDITED 0x200000
#define LOADER_DF_1_NORELOC 0x400000
#define LOADER_DF_1_SYMINTPOSE 0x800000
#define LOADER_DF_1_GLOBAUDIT 0x1000000
#define LOADER_DF_1_SINGLETON 0x2000000
#define LOADER_DF_1_STUB 0x4000000
#define LOADER_DF_1_PIE 0x8000000
#define LOADER_DF_1_KMOD 0x10000000
#define LOADER_DF_1_WEAKFILTER 0x20000000

/**
 * 动态表POSFLAG_1标签标志。
 */
#define LOADER_DF_P1_LAZYLOAD 0x1
#define LOADER_DF_P1_GROUPPERM 0x2
#define LOADER_DF_P1_DEFERRED 0x4
#define LOADER_DF_P1_EXISTING 0x8

#endif /*__AOS_UEFI_LOADER_INTERNAL_H__*/
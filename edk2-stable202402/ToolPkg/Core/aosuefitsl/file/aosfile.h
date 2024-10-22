/*
 * AOS Loader文件操作头文件。
 * 2024-10-15创建。 
 */
#ifndef __AOS_FILE_H__
#define __AOS_FILE_H__

#include "aosdefine.h"
#include <Protocol/LoadedImage.h>
#include <Library/FileHandleLib.h>

/*ELF文件对应偏移量*/
#define EI_MAG 0
#define EI_CLASS 4
#define EI_DATA 5
#define EI_OSABI 7

#define E_TYPE 16
#define E_MACHINE 18
#define E_VERSION 20
#define E_ENTRY 24
#define E_PHOFF 32
#define E_EHSIZE 52
#define E_PHNUM 56

#define P_TYPE 0
#define P_FLAGS 4
#define P_OFFSET 8
#define P_VADDR 16
#define P_FILESZ 32
#define P_MEMSZ 40

/*ELF对应需求属性常量*/
#define ELF_MAGIC 0x464C457F
#define ELF_CLASS_64 2
#define ELF_DATA_LSB 1
#define ELF_SYSTEMV_ABI 0

#define ELF_TYPE_DYN 3
#define ELF_MACHINE_AMD64 0x3E
#define ELF_VERSION_CURRENT 1
#define ELF_EHSIZE_64 64

#define ELF_P_TYPE_LOAD 1
#define ELF_PF_R 4
#define ELF_PF_W 2
#define ELF_PF_X 1
#define ELF_P_FLAGS_R ELF_PF_R
#define ELF_P_FLAGS_RX ELF_PF_R|ELF_PF_X
#define ELF_P_FLAGS_RW ELF_PF_R|ELF_PF_W

/*读取数组数据，小端序*/
#define aos_read_16(buffer,start) (LShiftU64(buffer[(start)+1],8)|(buffer[(start)+0]))
#define aos_read_32(buffer,start) (LShiftU64(buffer[(start)+3],24)|LShiftU64(buffer[(start)+2],16)|LShiftU64(buffer[(start)+1],8)|(buffer[(start)+0]))
#define aos_read_64(buffer,start) (LShiftU64(buffer[(start)+7],56)|LShiftU64(buffer[(start)+6],48)|LShiftU64(buffer[(start)+5],40)|LShiftU64(buffer[(start)+4],32)|LShiftU64(buffer[(start)+3],24)|LShiftU64(buffer[(start)+2],16)|LShiftU64(buffer[(start)+1],8)|(buffer[(start)+0]))

/*
 * 加载引导程序到内存上。
 */
EFI_STATUS
EFIAPI
aos_load_bootstrap(VOID);

#endif /*__AOS_FILE_H__*/
/**
 * AOS内核调试版本链接脚本。
 * @date 2025-12-14
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
ENTRY(aos_kernel_trampoline)

PHDRS
{
    text   PT_LOAD FLAGS(5); /*R-X*/
    rodata PT_LOAD FLAGS(4); /*R--*/
    data   PT_LOAD FLAGS(6); /*RW-*/
    bss    PT_LOAD FLAGS(6); /*RW-*/
}

SECTIONS
{
    . = SIZEOF_HEADERS;

    .text ALIGN(0x10) :
    {
        *(.text*)
    } :text

    . += 0x1000;

    .rodata ALIGN(0x10)  :
    {
        *(.rodata*)
    } :rodata

    . += 0x1000;

    .data ALIGN(0x10) :
    {
        *(.data*)
    } :data

    .bss ALIGN(0x10) (NOLOAD) :
    {
        *(.bss*)
    } :bss

    /DISCARD/ :
    {
        *(.note*)
        *(.eh_frame*)
        *(.eh_frame_hdr)
        *(.interp)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.hash)
        *(.gnu.hash)
    }
}
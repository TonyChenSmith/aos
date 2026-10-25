/**
 * 内核控制台字体库。
 * @date 2026-10-17
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
.file "fonts.S"

.section .rodata

/**
 * 控制台内嵌字体起始。
 */
.globl console_fonts_start
.p2align 4

console_fonts_start:
.incbin "../../research/kfonts/out/kfonts"

/**
 * 控制台内嵌字体结尾。
 */
.globl console_fonts_end
console_fonts_end:
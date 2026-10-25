/**
 * 内核字体库。
 * @date 2026-10-17
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
.file "kfonts.S"

.section .rodata

/**
 * 内核内嵌字体起始。
 */
.globl kfonts_header
.p2align 4

kfonts_header:
.incbin "../../research/kfonts/out/kfonts"

/**
 * 内核内嵌字体结尾。
 */
.globl kfonts_end
kfonts_end:
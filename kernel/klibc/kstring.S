/**
 * 内核类C标准库字符串功能函数实现。
 * @date 2025-12-06
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
.file "kstring.S"

/**
 * 内存区域复制。
 * 
 * @param s1 rdi 目标基址。
 * @param s2 rsi 源基址。
 * @param n  rdx 复制字节数。
 * 
 * @return 复制后的目标基址。
 */
.text
.globl kmemcpy
.p2align 4
.type kmemcpy,@function

kmemcpy:
    movq %rdx,%rcx
    movq %rdi,%rax
    cld
    rep movsb
    retq
.size kmemcpy,.-kmemcpy

/**
 * 内存区域可重叠复制。
 * 
 * @param s1 rdi 目标基址。
 * @param s2 rsi 源基址。
 * @param n  rdx 复制字节数。
 * 
 * @return 复制后的目标基址。
 */
.text
.globl kmemmove
.p2align 4
.type kmemmove,@function

kmemmove:
    movq %rdi,%rax

    testq %rdx,%rdx
    jz .Lkmemmove_done

    cmpq %rsi,%rdi
    je .Lkmemmove_done

    cmpq %rsi,%rdi
    jb .Lkmemmove_fcopy # dest<src，前向复制。

    addq %rdx,%rsi # dsct>src，后向复制。
    addq %rdx,%rdi
    decq %rsi
    decq %rdi

    std
    movq %rdx,%rcx
    rep movsb
    cld
    jmp .Lkmemmove_done

.Lkmemmove_fcopy:
    cld
    movq %rdx,%rcx
    rep movsb
.Lkmemmove_done:
    retq
.size kmemmove,.-kmemmove
/**
 * 内核内存操作库函数。
 * @date 2025-12-06
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
.file "memory.S"
.text

/**
 * 将源内存复制前n个字节到目的内存。源内存与目的内在前n字节内有重叠时复制结果不保证正确。
 * 
 * @param dest rdi 目的内存。
 * @param src  rsi 源内存。
 * @param n    rdx 复制字节数。
 * 
 * @return 返回目的内存的起始地址，便于进行链式处理。
 */
.globl memory_copy
.p2align 4
.type memory_copy,@function

memory_copy:
    mov %rdx,%rcx
    mov %rdi,%rax
    rep movsb
    ret
.size memory_copy,.-memory_copy

/**
 * 将源内存复制前n个字节到目的内存。这一函数允许复制时存在重叠情况。
 * 
 * @param dest rdi 目的内存。
 * @param src  rsi 源内存。
 * @param n    rdx 复制字节数。
 * 
 * @return 返回目的内存的起始地址，便于进行链式处理。
 */
.globl memory_move
.p2align 4
.type memory_move,@function

memory_move:
    mov %rdi,%rax
    test %rdx,%rdx
    jz .Lmemory_move_done
    cmp %rsi,%rdi
    je .Lmemory_move_done
    cmp %rsi,%rdi
    jb .Lmemory_move_fcopy # dest<src，前向复制。
    add %rdx,%rsi # dsst>src，后向复制。
    add %rdx,%rdi
    dec %rsi
    dec %rdi
    std
    mov %rdx,%rcx
    rep movsb
    cld
    jmp .Lmemory_move_done
.Lmemory_move_fcopy:
    mov %rdx,%rcx
    rep movsb
.Lmemory_move_done:
    ret
.size memory_move,.-memory_move

/**
 * 将内存m前n个字节设置成固定值。
 * 
 * @param m     rdi 内存m。
 * @param value rsi 设置值。
 * @param n     rdx 操作字节数。
 * 
 * @return 返回内存m的起始地址，便于进行链式处理。
 */
.globl memory_set
.p2align 4
.type memory_set,@function

memory_set:
    mov %sil,%al
    mov %rdi,%r8
    mov %rdx,%rcx
    rep stosb
    mov %r8,%rax
    ret
.size memory_set,.-memory_set

/**
 * 对内存a与内存b的前n个字节进行比较。
 * 
 * @param a rdi 内存a。
 * @param b rsi 内存b。
 * @param n rdx 比较字节数。
 * 
 * @return 相等返回真，不相等返回假。
 */
.globl memory_compare
.p2align 4
.type memory_compare,@function

memory_compare:
    xor %rax,%rax
    mov $1,%r8
    test %rdx,%rdx
    jz .Lmemory_compare_equal
    mov %rdx,%rcx
    repe cmpsb
.Lmemory_compare_equal:
    cmove %r8,%rax
    ret
.size memory_compare,.-memory_compare
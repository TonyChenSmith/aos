/**
 * 内核蹦床实现。
 * 模块入口，负责从模块“aos.uefi”跳转到模块“aos.kernel”。
 * @date 2025-12-04
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
.file "trampoline.S"

/**
 * 内核模块入口。
 * 
 * @param params rdi 启动参数。
 * 
 * @return 不再返回。
 */
.extern aos_kernel_entry

/**
 * 内核蹦床函数，在换栈后进入内核入口函数不再返回。
 * 
 * @param params rcx 启动参数。
 * @param stack  rdx 栈底。
 * 
 * @return 不再返回。
 */
.text
.globl aos_kernel_trampoline
.p2align 4
.type aos_kernel_trampoline,@function

aos_kernel_trampoline:
    movq %rcx,%rdi
    movq %rdx,%rsp
    jmp aos_kernel_entry
.size aos_kernel_trampoline,.-aos_kernel_trampoline
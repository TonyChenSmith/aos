/**
 * 内核蹦床实现。
 * 模块入口，负责从模块“aos.uefi”跳转到模块“aos.kernel”。
 * @date 2025-12-04
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
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
.text

/**
 * 内核蹦床函数，在换栈后进入内核入口函数不再返回。
 * 
 * @param params rcx 启动参数。
 * @param stack  rdx 栈底。
 * 
 * @return 不再返回。
 */
.globl aos_kernel_trampoline
.p2align 4
.type aos_kernel_trampoline,@function

aos_kernel_trampoline:
    movq   %rcx,%rdi
    movq   %rdx,%rsp
    cli
    cld
    /*将所有AP变成初始状态*/
    movzbl 0x17(%rdi),%esi
    testb  %sil,%sil
    je     .Ltrampoline_end
    cmpb   $0x1,%sil
    jne    .Lx2apic
    movl   $0x1b,%ecx
    rdmsr
    shlq   $0x20,%rdx
    andl   $0xfffff000,%eax
    movl   $0x0,0x310(%rdx,%rax)
    movl   $0xc4500,0x300(%rdx,%rax)
    mfence
    jmp    .Ltrampoline_end
.Lx2apic:
    movl    $0xc4500,%eax
    xorl    %edx,%edx
    movl    $0x830,%ecx
    wrmsr
.Ltrampoline_end:
    jmp aos_kernel_entry
.size aos_kernel_trampoline,.-aos_kernel_trampoline
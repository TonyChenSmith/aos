;
; ESP内核入口函数，负责EFI的MS ABI与SystemV ABI转接。
; @date 2024-11-02
;
; Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default REL
section .text

;------------------------------------------------------------------------------
; 初始化内存空间第一阶段。
;------------------------------------------------------------------------------
extern boot_init_memory_step1

;------------------------------------------------------------------------------
; 内核跳板函数。用于EFI到ESP内核的跳转，同时变换调用ABI。
;
; @define extern void aos_boot_trampoline(boot_params* param,const boot_base_functions* bbft);
;
; @param (rcx) param 启动参数结构指针。
; @param (rdx) bbft  基础模块函数表指针。
;
; @return 不返回。
;------------------------------------------------------------------------------
global aos_boot_trampoline
aos_boot_trampoline:
	mov rdi,rcx
	mov rsi,rdx
	call boot_init_memory_step1
	ret
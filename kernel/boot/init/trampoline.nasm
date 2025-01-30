;
; 启动核蹦床函数，负责EFI的MS ABI与SystemV ABI转接。
; @date 2025-01-16
;
; Copyright (c) 2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default rel
section .text

;------------------------------------------------------------------------------
; 初始化第一阶段。
;------------------------------------------------------------------------------
extern boot_init_step1

;------------------------------------------------------------------------------
; 启动核蹦床函数。用于EFI到启动核的跳转，同时变换调用ABI。
;
; @define extern void aos_boot_trampoline(boot_params* restrict params);
;
; @param (rcx) param 启动参数结构指针。
;
; @return 不返回。
;------------------------------------------------------------------------------
global aos_boot_trampoline
aos_boot_trampoline:
	mov rdi,rcx
	call boot_init_step1
	ret
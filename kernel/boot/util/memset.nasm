;
; 内存设置。行为类似于string.h的memset函数。
; @date 2025-01-17
;
; Copyright (c) 2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default rel
section .text

;------------------------------------------------------------------------------
; 内存设置。行为类似于string.h的memset函数。
;
; @param (rdi) 起始地址。
; @param (rsi) 写入值。
; @param (rdx) 写入数目。
; 
; @return 目的起始。
;------------------------------------------------------------------------------
global boot_memset
boot_memset:
	mov rcx,rdx
	mov rax,rsi
	mov rdx,rdi
	cld
	rep stosb
	mov rax,rdx
	ret
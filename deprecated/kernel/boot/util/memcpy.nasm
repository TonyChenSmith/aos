;
; 内存复制。行为类似于string.h的memcpy函数。
; @date 2025-01-17
;
; Copyright (c) 2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default rel
section .text

;------------------------------------------------------------------------------
; 内存复制。行为类似于string.h的memcpy函数。
;
; @param (rdi) 目的起始。
; @param (rsi) 源起始。
; @param (rdx) 复制数目。
; 
; @return 目的起始。
;------------------------------------------------------------------------------
global boot_memcpy
boot_memcpy:
	mov rcx,rdx
	mov rax,rdi
	cld
	rep movsb
	ret
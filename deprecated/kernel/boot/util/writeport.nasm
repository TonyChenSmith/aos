;
; 端口写入。
; @date 2025-01-17
;
; Copyright (c) 2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default rel
section .text

;------------------------------------------------------------------------------
; 端口写入。
;
; @param (rdi) 源起始。
; @param (rsi) 写入位宽。
; @param (rdx) 写入端口。
; @param (rcx) 写入数目。
; 
; @return 写入内存。
;------------------------------------------------------------------------------
global boot_writeport
boot_writeport:
	mov eax,esi
	mov rsi,rdi
	cld
.wb:
	cmp eax,0
	ja .ww
	rep outsb
	jmp .wend
.ww:
	cmp eax,1
	ja .wd
	rep outsw
	jmp .wend
.wd:
	cmp eax,2
	ja .wfault
	rep outsd
.wend:
	mov rax,rdi
	ret
.wfault:
	xor rax,rax
	ret

;
; 端口读取。
; @date 2025-01-17
;
; Copyright (c) 2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default rel
section .text

;------------------------------------------------------------------------------
; 端口读取。
;
; @param (rdi) 目的起始。
; @param (rsi) 读入位宽。
; @param (rdx) 读入端口。
; @param (rcx) 读取数目。
; 
; @return 输出内存。
;------------------------------------------------------------------------------
global boot_readport
boot_readport:
	mov eax,esi
	mov rsi,rdi
	cld
.rb:
	cmp eax,0
	ja .rw
	rep insb
	jmp .rend
.rw:
	cmp eax,1
	ja .rd
	rep insw
	jmp .rend
.rd:
	cmp eax,2
	ja .rfault
	rep insd
.rend:
	mov rax,rsi
	ret
.rfault:
	xor rax,rax
	ret

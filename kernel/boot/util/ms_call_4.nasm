;
; 对MS ABI四参数函数的调用。
; @date 2025-01-17
;
; Copyright (c) 2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default REL
section .text

;------------------------------------------------------------------------------
; 对MS ABI四参数函数的调用。
;
; @param (rdi) 被调用函数地址。
; @param (rsi) 被调用函数的第一个参数。
; @param (rdx) 被调用函数的第二个参数。
; @param (rcx) 被调用函数的第三个参数。
; @param (r8)  被调用函数的第四个参数。
; 
; @return 被调用函数的返回值。
;------------------------------------------------------------------------------
global boot_ms_call_4
boot_ms_call_4:
	mov rax,rdi
	mov r9,r8
	mov r8,rcx
	mov rcx,rsi
	call rax
	ret

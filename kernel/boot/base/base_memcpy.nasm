;
; 内存复制，行为类似于string.h的memcpy函数。
; @date 2024-10-28
;
default REL
section .text

;------------------------------------------------------------------------------
; 内存复制，行为类似于string.h的memcpy函数。
;
; @param (rdi) 输出内存。
; @param (rsi) 输入内存。
; @param (rdx) 复制字节数。
; 
; @return 输出内存。
;------------------------------------------------------------------------------
global base_memcpy
base_memcpy:
	mov rcx,rdx
	mov rax,rdi
	cld
	rep movsb
	ret
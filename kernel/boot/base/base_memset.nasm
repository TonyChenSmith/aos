;
; 内存设置，行为类似于string.h的memset函数。
; @date 2024-12-08
;
default REL
section .text

;------------------------------------------------------------------------------
; 内存设置，行为类似于string.h的memset函数。
;
; @param (rdi) 写入内存。
; @param (rsi) 写入值。
; @param (rdx) 复制字节数。
; 
; @return 写入内存。
;------------------------------------------------------------------------------
global base_memset
base_memset:
	mov rcx,rdx
	mov rax,rsi
	mov rdx,rdi
	cld
	rep stosb
	mov rax,rdx
	ret
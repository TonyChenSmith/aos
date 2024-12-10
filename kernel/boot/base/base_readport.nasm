;
; 端口读取。
; @date 2024-12-08
;
default REL
section .text

;------------------------------------------------------------------------------
; 端口读取。
;
; @param (rdi) 输出内存。
; @param (rsi) 。
; @param (rdx) 写入端口。
; @param (rcx) 读取字节数。
; 
; @return 输出内存。
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
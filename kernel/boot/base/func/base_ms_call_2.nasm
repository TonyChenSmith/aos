;
; 对MS ABI两参数函数的调用函数定义。
; @date 2024-10-27
;
default REL
section .text

;------------------------------------------------------------------------------
; 对MS ABI两参数函数的调用。
;
; @param (rdi) 被调用函数地址。
; @param (rsi) 被调用函数的第一个参数。
; @param (rdx) 被调用函数的第二个参数。
; 
; @return 被调用函数的返回值。
;------------------------------------------------------------------------------
global base_ms_call_2
base_ms_call_2:
	mov rax,rdi
	mov rcx,rsi
	call rax
	ret

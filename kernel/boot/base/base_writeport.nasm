;
; 端口写入。
; @date 2024-12-10
;
default REL
section .text

;------------------------------------------------------------------------------
; 端口写入。
;
; @param (rdi) 写入内存。
; @param (rsi) 写入位宽。
; @param (rdx) 写入端口。
; @param (rcx) 写入长度。
; 
; @return 写入内存。
;------------------------------------------------------------------------------
global base_writeport
base_writeport:
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

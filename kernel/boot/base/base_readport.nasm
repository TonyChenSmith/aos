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
; @param (rsi) 读入位宽。
; @param (rdx) 读入端口。
; @param (rcx) 读取长度。
; 
; @return 输出内存。
;------------------------------------------------------------------------------
global base_readport
base_readport:
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

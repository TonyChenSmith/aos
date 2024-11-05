;
; 测试汇编代码，负责打印。
; 2024-10-19
;
default REL
section .text

;打印字符。
;extern void print_bytes(const char* buf,uint64 size)
;buf rdi
;size rsi
global print_bytes
print_bytes:
	xchg rdi,rsi
	mov rcx,rdi
	mov rdx,0x402 ;debugcon
	cld
	rep outsb
	ret

;设置代码段。
global aos_set_cs
aos_set_cs:
    push rcx
    lea rax, [.set_cs_long_jump]
    push rax
    retfq
.set_cs_long_jump:
    ret

;设置数据段。
global aos_set_ds
aos_set_ds:
o16 mov ss,cx
o16 mov ds,cx
o16 mov es,cx
o16 mov fs,cx
o16 mov gs,cx
    ret

global aos_cpu_hlt
aos_cpu_hlt:
	hlt
    ret
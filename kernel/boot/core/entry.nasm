;
; 入口汇编代码，负责EFI的MS ABI与SystemV ABI转接。
; 2024-10-18
;
default REL
section .text

;------------------------------------------------------------------------------
; 内核入口函数。
;------------------------------------------------------------------------------
extern aos_bootstrap_entry

;------------------------------------------------------------------------------
; 内核跳板函数。用于EFI到AOS的跳转，同时变换调用ABI。
;
; @define extern int aos_bootstrap_jump(void* boot_params,void* unused);
;
; @
;	(rcx) boot_params 启动参数结构指针。
;	(rdx) unused	  未使用。
;------------------------------------------------------------------------------
global aos_bootstrap_jump
aos_bootstrap_jump:
	mov rdi,rcx
	mov rsi,rdx
	call aos_bootstrap_entry
	ret
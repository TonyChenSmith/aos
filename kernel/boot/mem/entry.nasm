;
; ESP内核入口函数，负责EFI的MS ABI与SystemV ABI转接。
; @date 2024-11-02
;
default REL
section .text

;------------------------------------------------------------------------------
; 声明内存模块初始化函数。
;------------------------------------------------------------------------------
extern boot_init_memory

;------------------------------------------------------------------------------
; 内核跳板函数。用于EFI到ESP内核的跳转，同时变换调用ABI。
;
; @define extern void aos_boot_trampoline(const boot_params* boot_params,const boot_base_functions* base_func);
;
; @param (rcx) boot_params 启动参数结构指针。
; @param (rdx) base_func   基础模块函数表指针。
;
; @return 不返回。
;------------------------------------------------------------------------------
global aos_boot_trampoline
aos_boot_trampoline:
	mov rdi,rcx
	mov rsi,rdx
	call boot_init_memory
	ret
;
; 基础模块初始化跳板，转接MS ABI和System ABI。
; @date 2024-11-03
;
default REL
section .text

;------------------------------------------------------------------------------
; 声明基础模块初始化函数。
;------------------------------------------------------------------------------
extern boot_init_base

;------------------------------------------------------------------------------
; 基础模块初始化函数跳板。
;
; @define extern boot_base_functions* aos_boot_base_trampoline(const boot_params* params);
;
; @return 基础模块函数表指针。
;------------------------------------------------------------------------------
global aos_boot_base_trampoline
aos_boot_base_trampoline:
	mov rdi,rcx
	call boot_init_base
	ret
;
; 初始化内存空间第二阶段。负责切换运行栈。
; @date 2024-12-27
;
; Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
default REL
section .text

;------------------------------------------------------------------------------
; 初始化内存空间第一阶段。
;------------------------------------------------------------------------------
extern boot_init_memory_step3

;------------------------------------------------------------------------------
; 初始化内存空间第二阶段。负责切换运行栈。
;
; @define extern void boot_init_memory_step2(boot_params* param,const boot_base_functions* bbft,const uintn stack);
;
; @param (rdi) param 启动参数结构指针。
; @param (rsi) bbft  基础模块函数表指针。
; @param (rdx) stack 栈基址。
;
; @return 不返回。
;------------------------------------------------------------------------------
global boot_init_memory_step2
boot_init_memory_step2:
	mov rsp,rdx
	jmp boot_init_memory_step3
	ret
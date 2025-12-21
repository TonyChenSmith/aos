;------------------------------------------------------------------------------
;* Assembly functions required for module page tables and linear area 
;* management.
;* @date 2025-12-21
;* 
;* Copyright (c) 2025 Tony Chen Smith
;* 
;* SPDX-License-Identifier: MIT
;------------------------------------------------------------------------------
DEFAULT REL
SECTION .text

;------------------------------------------------------------------------------
;* Set the code segment register.
;* 
;* @param cs rcx Code segment selector.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
global ASM_PFX(set_cs)
ASM_PFX(set_cs):
    push  rcx
    lea   rax,[.Lcs_long_jump]
    push  rax
    retfq
.Lcs_long_jump:
    ret

;------------------------------------------------------------------------------
;* Set the data segment register.
;* 
;* @param ds rcx Data segment selector.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
global ASM_PFX(set_ds)
ASM_PFX(set_ds):
    mov ds,cx
    mov es,cx
    mov fs,cx
    mov gs,cx
    mov ss,cx
    ret
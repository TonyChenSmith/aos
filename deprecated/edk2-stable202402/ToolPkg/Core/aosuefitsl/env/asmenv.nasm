;
; Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
;
; SPDX-License-Identifier: MIT
;
DEFAULT REL
SECTION .text

;set cs
global ASM_PFX(aos_set_cs)
ASM_PFX(aos_set_cs):
    push rcx
    lea rax, [set_cs_long_jump]
    push rax
    retfq
set_cs_long_jump:
    ret

;set ds
global ASM_PFX(aos_set_ds)
ASM_PFX(aos_set_ds):
o16 mov ss,cx
o16 mov ds,cx
o16 mov es,cx
o16 mov fs,cx
o16 mov gs,cx
    ret
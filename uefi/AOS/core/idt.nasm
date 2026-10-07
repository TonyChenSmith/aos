;------------------------------------------------------------------------------
;* Assembly functions for the IDT and the low-level debug output.
;* @date 2026-09-17
;* 
;* Copyright (c) 2026 Tony Chen Smith
;* 
;* SPDX-License-Identifier: MIT
;------------------------------------------------------------------------------
DEFAULT REL
SECTION .rdata

;------------------------------------------------------------------------------
; QEMU isa-debugcon data port. test/qemu.bat redirects it to debugcon.log.
;------------------------------------------------------------------------------
%define AOS_QEMU_DEBUG_PORT 0x402

;------------------------------------------------------------------------------
; The start address of the IDT handler code.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_BOOT_IDT_DATA)
ASM_PFX(AOS_BOOT_IDT_DATA):
ASM_PFX(AOS_BOOT_IDT_DATA_BEGIN):

;------------------------------------------------------------------------------
;* Park the current CPU.
;* 
;* All exception handlers end here until a complete panic path exists, so a
;* faulted state cannot be escaped by accident.
;* 
;* @return No return.
;------------------------------------------------------------------------------
ASM_PFX(idt_cpu_halt):
    cli
.Lsleep_loop:
    hlt
    jmp .Lsleep_loop

;------------------------------------------------------------------------------
;* Write a ASCII character to the QEMU debug port.
;* 
;* @param rcx ASCII character value.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_qemu_debugcon_debug_putc):
    mov al,cl
    mov dx,AOS_QEMU_DEBUG_PORT
    out dx,al
    ret

;------------------------------------------------------------------------------
;* Write a NUL terminated ASCII string to the QEMU debug port.
;* 
;* @param rcx String address.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_qemu_debugcon_debug_puts):
    push  rsi
    mov   rsi,rcx
    mov   dx,AOS_QEMU_DEBUG_PORT
    cld
.Lputs_loop:
    lodsb
    test  al,al
    jz    .Lputs_done
    out   dx,al
    jmp   .Lputs_loop
.Lputs_done:
    pop   rsi
    ret

;------------------------------------------------------------------------------
;* Write the low byte of RCX as two hexadecimal characters.
;* 
;* @param rcx Output value, only the low 8 bits are used.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_qemu_debugcon_debug_put_hex_byte):
    mov  dx,AOS_QEMU_DEBUG_PORT
    mov  al,cl
    shr  al,4
    call .Lput_nibble
    mov  al,cl
    call .Lput_nibble
    ret
.Lput_nibble:
    and  al,0x0F
    cmp  al,10
    jb   .Ldigit
    add  al,'A'-10
    jmp  .Lout
.Ldigit:
    add  al,'0'
.Lout:
    out  dx,al
    ret

;------------------------------------------------------------------------------
;* Write the low word of RCX as four hexadecimal characters.
;* 
;* @param rcx Output value, only the low 16 bits are used.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_qemu_debugcon_debug_put_hex_word):
    mov  dx,AOS_QEMU_DEBUG_PORT
    mov  ax,cx
    shr  ax,12
    call .Lput_nibble
    mov  ax,cx
    shr  ax,8
    call .Lput_nibble
    mov  al,cl
    shr  al,4
    call .Lput_nibble
    mov  al,cl
    call .Lput_nibble
    ret
.Lput_nibble:
    and  al,0x0F
    cmp  al,10
    jb   .Ldigit
    add  al,'A'-10
    jmp  .Lout
.Ldigit:
    add  al,'0'
.Lout:
    out  dx,al
    ret

;------------------------------------------------------------------------------
;* Write the low dword of RCX as eight hexadecimal characters.
;* 
;* @param rcx Output value, only the low 32 bits are used.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword):
    mov  dx,AOS_QEMU_DEBUG_PORT
    mov  eax,ecx
    shr  eax,28
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,24
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,20
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,16
    call .Lput_nibble
    mov  ax,cx
    shr  ax,12
    call .Lput_nibble
    mov  ax,cx
    shr  ax,8
    call .Lput_nibble
    mov  al,cl
    shr  al,4
    call .Lput_nibble
    mov  al,cl
    call .Lput_nibble
    ret
.Lput_nibble:
    and  al,0x0F
    cmp  al,10
    jb   .Ldigit
    add  al,'A'-10
    jmp  .Lout
.Ldigit:
    add  al,'0'
.Lout:
    out  dx,al
    ret

;------------------------------------------------------------------------------
;* Write the qword of RCX as sixteen hexadecimal characters.
;* 
;* @param rcx Output value.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_qemu_debugcon_debug_put_hex_qword):
    mov  dx,AOS_QEMU_DEBUG_PORT
    mov  rax,rcx
    shr  rax,60
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,56
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,52
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,48
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,44
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,40
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,36
    call .Lput_nibble
    mov  rax,rcx
    shr  rax,32
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,28
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,24
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,20
    call .Lput_nibble
    mov  eax,ecx
    shr  eax,16
    call .Lput_nibble
    mov  ax,cx
    shr  ax,12
    call .Lput_nibble
    mov  ax,cx
    shr  ax,8
    call .Lput_nibble
    mov  al,cl
    shr  al,4
    call .Lput_nibble
    mov  al,cl
    call .Lput_nibble
    ret
.Lput_nibble:
    and  al,0x0F
    cmp  al,10
    jb   .Ldigit
    add  al,'A'-10
    jmp  .Lout
.Ldigit:
    add  al,'0'
.Lout:
    out  dx,al
    ret

;------------------------------------------------------------------------------
; Messages of the interrupt report.
;------------------------------------------------------------------------------
ASM_PFX(idt_report_csrip):
    db "[aos.uefi] CS:RIP = ",0
ASM_PFX(idt_report_ssrsp):
    db "[aos.uefi] SS:RSP = ",0
ASM_PFX(idt_report_rflags):
    db "[aos.uefi] RFLAGS = ",0
ASM_PFX(idt_report_error):
    db "[aos.uefi] ERROR  = ",0
ASM_PFX(idt_report_cr2):
    db "[aos.uefi] CR2    = 0x",0

;------------------------------------------------------------------------------
; Messages of the external interrupt report.
;------------------------------------------------------------------------------
ASM_PFX(idt_external_message_head):
    db "[aos.uefi] external interrupt 0x",0
ASM_PFX(idt_external_message_tail):
    db ".\n",0

;------------------------------------------------------------------------------
; Messages of the 32 architectural vectors. One message per vector, all NUL
; terminated as required by idt_qemu_debugcon_debug_puts.
;------------------------------------------------------------------------------
ASM_PFX(idt_0_message):
    db "[aos.uefi] INT 0 (#DE).\n",0
ASM_PFX(idt_1_message):
    db "[aos.uefi] INT 1 (#DB).\n",0
ASM_PFX(idt_2_message):
    db "[aos.uefi] INT 2 (NMI).\n",0
ASM_PFX(idt_3_message):
    db "[aos.uefi] INT 3 (#BP).\n",0
ASM_PFX(idt_4_message):
    db "[aos.uefi] INT 4 (#OF).\n",0
ASM_PFX(idt_5_message):
    db "[aos.uefi] INT 5 (#BR).\n",0
ASM_PFX(idt_6_message):
    db "[aos.uefi] INT 6 (#UD).\n",0
ASM_PFX(idt_7_message):
    db "[aos.uefi] INT 7 (#NM).\n",0
ASM_PFX(idt_8_message):
    db "[aos.uefi] INT 8 (#DF).\n",0
ASM_PFX(idt_9_message):
    db "[aos.uefi] INT 9 (Coprocessor-Segment-Overrun Exception).\n",0
ASM_PFX(idt_10_message):
    db "[aos.uefi] INT 10 (#TS).\n",0
ASM_PFX(idt_11_message):
    db "[aos.uefi] INT 11 (#NP).\n",0
ASM_PFX(idt_12_message):
    db "[aos.uefi] INT 12 (#SS).\n",0
ASM_PFX(idt_13_message):
    db "[aos.uefi] INT 13 (#GP).\n",0
ASM_PFX(idt_14_message):
    db "[aos.uefi] INT 14 (#PF).\n",0
ASM_PFX(idt_15_message):
    db "[aos.uefi] INT 15 (Reserved).\n",0
ASM_PFX(idt_16_message):
    db "[aos.uefi] INT 16 (#MF).\n",0
ASM_PFX(idt_17_message):
    db "[aos.uefi] INT 17 (#AC).\n",0
ASM_PFX(idt_18_message):
    db "[aos.uefi] INT 18 (#MC).\n",0
ASM_PFX(idt_19_message):
    db "[aos.uefi] INT 19 (#XM).\n",0
ASM_PFX(idt_20_message):
    db "[aos.uefi] INT 20 (#VE).\n",0
ASM_PFX(idt_21_message):
    db "[aos.uefi] INT 21 (#CP).\n",0
ASM_PFX(idt_22_message):
    db "[aos.uefi] INT 22 (Reserved).\n",0
ASM_PFX(idt_23_message):
    db "[aos.uefi] INT 23 (Reserved).\n",0
ASM_PFX(idt_24_message):
    db "[aos.uefi] INT 24 (Reserved).\n",0
ASM_PFX(idt_25_message):
    db "[aos.uefi] INT 25 (Reserved).\n",0
ASM_PFX(idt_26_message):
    db "[aos.uefi] INT 26 (Reserved).\n",0
ASM_PFX(idt_27_message):
    db "[aos.uefi] INT 27 (Reserved).\n",0
ASM_PFX(idt_28_message):
    db "[aos.uefi] INT 28 (#HV).\n",0
ASM_PFX(idt_29_message):
    db "[aos.uefi] INT 29 (#VC).\n",0
ASM_PFX(idt_30_message):
    db "[aos.uefi] INT 30 (#SX).\n",0
ASM_PFX(idt_31_message):
    db "[aos.uefi] INT 31 (Reserved).\n",0

;------------------------------------------------------------------------------
;* Report an interrupt vector to the QEMU debug port.
;* 
;* @param rcx Message address.
;* @param rdx Interrupt stack frame.
;* 
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_common):
    mov  r8,rdx
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[r8+8]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_word)
    mov  rcx,':'
    call ASM_PFX(idt_qemu_debugcon_debug_putc)
    mov  rcx,[r8]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_qword)
    mov  rcx,'\n'
    call ASM_PFX(idt_qemu_debugcon_debug_putc)
    lea  rcx,[rel ASM_PFX(idt_report_ssrsp)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[r8+32]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_word)
    mov  rcx,':'
    call ASM_PFX(idt_qemu_debugcon_debug_putc)
    mov  rcx,[r8+24]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_qword)
    mov  rcx,'\n'
    call ASM_PFX(idt_qemu_debugcon_debug_putc)
    lea  rcx,[rel ASM_PFX(idt_report_rflags)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[r8+16]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_qword)
    mov  rcx,'\n'
    call ASM_PFX(idt_qemu_debugcon_debug_putc)
    ret

;------------------------------------------------------------------------------
;* Report an external interrupt vector to the QEMU debug port.
;*
;* @param rcx Vector number.
;* @param rdx Interrupt stack frame.
;*
;* @return No return value.
;------------------------------------------------------------------------------
ASM_PFX(idt_external_common):
    mov  r8,rdx
    mov  r9,rcx
    lea  rcx,[rel ASM_PFX(idt_external_message_head)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,r9
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_byte)
    lea  rcx,[rel ASM_PFX(idt_external_message_tail)]
    mov  rdx,r8
    call ASM_PFX(idt_common)
    ret

;------------------------------------------------------------------------------
; Entry points of the 32 architectural vectors. Each one loads its message
; and jumps to the shared tail. Exported for the IDT installation.
;------------------------------------------------------------------------------
ASM_PFX(idt_0_handler):
    lea  rcx,[rel ASM_PFX(idt_0_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_1_handler):
    lea  rcx,[rel ASM_PFX(idt_1_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_2_handler):
    push  rax
    push  rcx
    push  rdx
    push  r8
    push  r9
    lea   rcx,[rel ASM_PFX(idt_2_message)]
    lea   rdx,[rsp+32]
    call  ASM_PFX(idt_common)
    pop   r9
    pop   r8
    pop   rdx
    pop   rcx
    pop   rax
    iretq

ASM_PFX(idt_3_handler):
    lea  rcx,[rel ASM_PFX(idt_3_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_4_handler):
    lea  rcx,[rel ASM_PFX(idt_4_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_5_handler):
    lea  rcx,[rel ASM_PFX(idt_5_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_6_handler):
    lea  rcx,[rel ASM_PFX(idt_6_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_7_handler):
    lea  rcx,[rel ASM_PFX(idt_7_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_8_handler):
    lea  rcx,[rel ASM_PFX(idt_8_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_9_handler):
    lea  rcx,[rel ASM_PFX(idt_9_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_10_handler):
    lea  rcx,[rel ASM_PFX(idt_10_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_11_handler):
    lea  rcx,[rel ASM_PFX(idt_11_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_12_handler):
    lea  rcx,[rel ASM_PFX(idt_12_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_13_handler):
    lea  rcx,[rel ASM_PFX(idt_13_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_14_handler):
    lea  rcx,[rel ASM_PFX(idt_14_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    lea  rcx,[rel ASM_PFX(idt_report_cr2)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,cr2
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_qword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_15_handler):
    lea  rcx,[rel ASM_PFX(idt_15_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_16_handler):
    lea  rcx,[rel ASM_PFX(idt_16_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_17_handler):
    lea  rcx,[rel ASM_PFX(idt_17_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_18_handler):
    lea  rcx,[rel ASM_PFX(idt_18_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_19_handler):
    lea  rcx,[rel ASM_PFX(idt_19_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_20_handler):
    lea  rcx,[rel ASM_PFX(idt_20_message)]
    mov  rdx,rsp
    call ASM_PFX(idt_common)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_21_handler):
    lea  rcx,[rel ASM_PFX(idt_21_message)]
    lea  rdx,[rsp+8]
    call ASM_PFX(idt_common)
    lea  rcx,[rel ASM_PFX(idt_report_error)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    mov  rcx,[rsp]
    call ASM_PFX(idt_qemu_debugcon_debug_put_hex_dword)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_22_handler):
    lea  rcx,[rel ASM_PFX(idt_22_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_23_handler):
    lea  rcx,[rel ASM_PFX(idt_23_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_24_handler):
    lea  rcx,[rel ASM_PFX(idt_24_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_25_handler):
    lea  rcx,[rel ASM_PFX(idt_25_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_26_handler):
    lea  rcx,[rel ASM_PFX(idt_26_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_27_handler):
    lea  rcx,[rel ASM_PFX(idt_27_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_28_handler):
    lea  rcx,[rel ASM_PFX(idt_28_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_29_handler):
    lea  rcx,[rel ASM_PFX(idt_29_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_30_handler):
    lea  rcx,[rel ASM_PFX(idt_30_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

ASM_PFX(idt_31_handler):
    lea  rcx,[rel ASM_PFX(idt_31_message)]
    call ASM_PFX(idt_qemu_debugcon_debug_puts)
    jmp  ASM_PFX(idt_cpu_halt)

;------------------------------------------------------------------------------
; External vector stubs: load the vector number into RCX, call the reporting
; function, then park the CPU. The %rep block generates idt_32_handler ..
; idt_255_handler.
;------------------------------------------------------------------------------
%assign idt_vector 32
%rep 224
ASM_PFX(idt_ %+ idt_vector %+ _handler):
    mov  ecx,%[idt_vector]
    mov  rdx,rsp
    call ASM_PFX(idt_external_common)
    jmp  ASM_PFX(idt_cpu_halt)
%assign idt_vector idt_vector+1
%endrep

;------------------------------------------------------------------------------
; The end address of the IDT handler code.
;------------------------------------------------------------------------------
ASM_PFX(AOS_BOOT_IDT_DATA_END):

;------------------------------------------------------------------------------
; The array of the IDT handler offsets.
;------------------------------------------------------------------------------
align 8
global ASM_PFX(AOS_BOOT_IDT_HANDLER_OFFSET)
ASM_PFX(AOS_BOOT_IDT_HANDLER_OFFSET):
%assign idt_vector 0
%rep 256
    dq idt_ %+ idt_vector %+ _handler-ASM_PFX(AOS_BOOT_IDT_DATA_BEGIN)
%assign idt_vector idt_vector+1
%endrep

;------------------------------------------------------------------------------
; The size of the IDT handler code.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_BOOT_IDT_DATA_SIZE)
ASM_PFX(AOS_BOOT_IDT_DATA_SIZE):
    dq ASM_PFX(AOS_BOOT_IDT_DATA_END)-ASM_PFX(AOS_BOOT_IDT_DATA_BEGIN)
.text
.globl print_bytes
.p2align 4
.type print_bytes,@function

print_bytes:
    xchg %rdi,%rsi
    mov %rdi,%rcx
    mov $0x402,%rdx
    cld
    rep outsb
    ret

.globl sleep
.p2align 4
.type sleep,@function

sleep:
    hlt
    ret
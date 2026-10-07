;------------------------------------------------------------------------------
;* These X.509 certificates of the public keys are used in ECDSA P-384
;* computations.
;* @date 2026-10-03
;* 
;* Copyright (c) 2026 Tony Chen Smith
;* 
;* SPDX-License-Identifier: MIT
;------------------------------------------------------------------------------
DEFAULT REL
SECTION .rdata

%ifdef AOS_NDEBUG

;------------------------------------------------------------------------------
; The "aos.kernel" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_KERNEL_PUBLIC_KEY)
ASM_PFX(AOS_KERNEL_PUBLIC_KEY):
incbin "../../../pack/sign/keys/kernel.release.public.der"
ASM_PFX(AOS_KERNEL_PUBLIC_KEY_END):

;------------------------------------------------------------------------------
; The size of the "aos.kernel" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_KERNEL_PUBLIC_KEY_SIZE)
ASM_PFX(AOS_KERNEL_PUBLIC_KEY_SIZE):
    dq ASM_PFX(AOS_KERNEL_PUBLIC_KEY_END)-ASM_PFX(AOS_KERNEL_PUBLIC_KEY)

;------------------------------------------------------------------------------
; The "aos.uefi" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_UEFI_PUBLIC_KEY)
ASM_PFX(AOS_UEFI_PUBLIC_KEY):
incbin "../../../pack/sign/keys/uefi.release.public.der"
ASM_PFX(AOS_UEFI_PUBLIC_KEY_END):

;------------------------------------------------------------------------------
; The size of the "aos.uefi" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_UEFI_PUBLIC_KEY_SIZE)
ASM_PFX(AOS_UEFI_PUBLIC_KEY_SIZE):
    dq ASM_PFX(AOS_UEFI_PUBLIC_KEY_END)-ASM_PFX(AOS_UEFI_PUBLIC_KEY)

%else

;------------------------------------------------------------------------------
; The "aos.kernel" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_KERNEL_PUBLIC_KEY)
ASM_PFX(AOS_KERNEL_PUBLIC_KEY):
incbin "../../../pack/sign/keys/kernel.debug.public.der"
ASM_PFX(AOS_KERNEL_PUBLIC_KEY_END):

;------------------------------------------------------------------------------
; The size of the "aos.kernel" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_KERNEL_PUBLIC_KEY_SIZE)
ASM_PFX(AOS_KERNEL_PUBLIC_KEY_SIZE):
    dq ASM_PFX(AOS_KERNEL_PUBLIC_KEY_END)-ASM_PFX(AOS_KERNEL_PUBLIC_KEY)

;------------------------------------------------------------------------------
; The "aos.uefi" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_UEFI_PUBLIC_KEY)
ASM_PFX(AOS_UEFI_PUBLIC_KEY):
incbin "../../../pack/sign/keys/uefi.debug.public.der"
ASM_PFX(AOS_UEFI_PUBLIC_KEY_END):

;------------------------------------------------------------------------------
; The size of the "aos.uefi" certificate.
;------------------------------------------------------------------------------
global ASM_PFX(AOS_UEFI_PUBLIC_KEY_SIZE)
ASM_PFX(AOS_UEFI_PUBLIC_KEY_SIZE):
    dq ASM_PFX(AOS_UEFI_PUBLIC_KEY_END)-ASM_PFX(AOS_UEFI_PUBLIC_KEY)

%endif
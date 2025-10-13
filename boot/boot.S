/* Multiboot2 header + 32-bit entry, GAS syntax, Intel mnemonics */
.intel_syntax noprefix
.code32

/* ---------- Multiboot2 header ---------- */
.section .multiboot, "a"
.align 8
mb2_header:
    .long 0xE85250D6            /* magic */
    .long 0                     /* arch = i386 */
    .long mb2_header_end - mb2_header
    .long -(0xE85250D6 + 0 + (mb2_header_end - mb2_header))  /* checksum */

    /* --- request: framebuffer (type=5) --- */
    .align 8
    .long 5                     /* type = 5 (framebuffer request) */
    .long 20                    /* size = 20 bytes */
    .long 1920                  /* preferred width  (GRUB может дать ближайшее) */
    .long 1080                  /* preferred height */
    .long 32                    /* preferred bpp = 32 (ARGB/RGBX) */

    /* required end tag */
    .align 8
    .short 0                    /* type = 0 (end) */
    .short 0
    .long 8
mb2_header_end:

/* ---------- Stack ---------- */
.section .bss, "aw", @nobits
.align 16
.global stack_top
stack_bottom:
    .space 16384
stack_top:

/* ---------- Entry ---------- */
.section .text, "ax"
.global _start
.extern kmain

_start:
    cli
    mov     esp, stack_top      /* known-good stack */

    /* GRUB: EAX=magic, EBX=mbi pointer.
       Call kmain(uint32_t magic, uint32_t info). */
    push    ebx
    push    eax
    call    kmain

.hang:
    hlt
    jmp     .hang

/* linker.ld */
ENTRY(_start)

SECTIONS {
  . = 2M;
  _kernel_start = .;
  .multiboot : {
    KEEP(*(.multiboot))
  }

  .text : ALIGN(4K) {
    *(.multiboot)
    *(.text*)
  }

  .rodata : ALIGN(4K) { *(.rodata*) }
  .data   : ALIGN(4K) { *(.data*)   }
  .bss    : ALIGN(4K) { *(.bss*)    *(COMMON) }
  _kernel_end = .;
}

/* linker.ld - Tells GCC how to arrange our kernel in memory */

ENTRY(_start)

SECTIONS {
    . = 1M;             /* Load kernel at 1MB mark */

    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)   /* Multiboot header must be first */
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }
}
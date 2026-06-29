ENTRY(_start)

SECTIONS {
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
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

    _kernel_end = .;    /* ← add this */

    /DISCARD/ : {
        *(.note.gnu.build-id)
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.comment)
    }
}
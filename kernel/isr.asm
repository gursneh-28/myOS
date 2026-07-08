[BITS 32]

; Macro for ISR without error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

; Macro for ISR with error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

; Macro for IRQ handlers
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; CPU Exceptions
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15

; Syscall (int 0x80) — use dword, 128 doesn't fit in signed byte
global isr128
isr128:
    cli
    push dword 0
    push dword 128
    jmp isr_common_stub

; Hardware IRQs
IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

extern isr_handler
isr_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call isr_handler
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

extern irq_handler
irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call irq_handler
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

section .note.GNU-stack noalloc noexec nowrite progbits
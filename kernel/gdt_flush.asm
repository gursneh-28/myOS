[BITS 32]

global gdt_flush
gdt_flush:
    mov eax, [esp+4]    ; Get pointer to GDT
    lgdt [eax]          ; Load GDT

    mov ax, 0x10        ; 0x10 = data segment offset in GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush     ; 0x08 = code segment, far jump to reload CS
.flush:
    ret

global idt_flush
idt_flush:
    mov eax, [esp+4]    ; Get pointer to IDT
    lidt [eax]          ; Load IDT
    ret
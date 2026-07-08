[BITS 32]

global enter_usermode_asm

; void enter_usermode_asm(uint32_t user_cs, uint32_t user_ds, uint32_t user_esp, uint32_t func)
enter_usermode_asm:
    mov ebp, esp
    mov eax, [ebp+8]   ; user_ds
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebp+8]   ; ss = user_ds
    push dword [ebp+12]  ; esp
    pushf
    pop eax
    or eax, 0x200        ; enable interrupts
    push eax             ; eflags
    push dword [ebp+4]   ; cs = user_cs
    push dword [ebp+16]  ; eip = func

    iret

section .note.GNU-stack noalloc noexec nowrite progbits
; boot.asm - This is the first code that runs when our OS boots
; BIOS loads this at memory address 0x7C00

[BITS 16]           ; We start in 16-bit "Real Mode" (how x86 CPUs boot)
[ORG 0x7C00]        ; Tell assembler our code starts at address 0x7C00

start:
    ; Set up segment registers
    xor ax, ax      ; ax = 0
    mov ds, ax      ; data segment = 0
    mov es, ax      ; extra segment = 0
    mov ss, ax      ; stack segment = 0
    mov sp, 0x7C00  ; stack grows downward from where we loaded

    ; Print "Hello from myOS!" using BIOS interrupt
    mov si, msg     ; si points to our message
.print_loop:
    lodsb           ; load next character from si into al
    or al, al       ; check if character is 0 (end of string)
    jz .done        ; if zero, we're done
    mov ah, 0x0E    ; BIOS teletype function
    int 0x10        ; call BIOS video interrupt
    jmp .print_loop ; next character

.done:
    cli             ; disable interrupts
    hlt             ; halt the CPU

msg db "Hello from myOS!", 0  ; our message, null terminated

; Boot sector MUST be exactly 512 bytes
; Pad with zeros until byte 510, then add boot signature
times 510 - ($ - $$) db 0
dw 0xAA55           ; Magic number - tells BIOS this is bootable!

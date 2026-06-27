CC = gcc
AS = nasm
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -nodefaultlibs -no-pie -fno-pic

all: myos.iso

kernel.elf: boot/multiboot.asm kernel/kernel.c kernel/shell.c kernel/gdt.c kernel/idt.c kernel/isr.c kernel/gdt_flush.asm kernel/isr.asm drivers/vga.c drivers/keyboard.c
	$(AS) -f elf32 boot/multiboot.asm -o boot/multiboot.o
	$(AS) -f elf32 kernel/gdt_flush.asm -o kernel/gdt_flush.o
	$(AS) -f elf32 kernel/isr.asm -o kernel/isr_asm.o
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	$(CC) $(CFLAGS) -c kernel/shell.c -o kernel/shell.o
	$(CC) $(CFLAGS) -c kernel/gdt.c -o kernel/gdt.o
	$(CC) $(CFLAGS) -c kernel/idt.c -o kernel/idt.o
	$(CC) $(CFLAGS) -c kernel/isr.c -o kernel/isr.o
	$(CC) $(CFLAGS) -c drivers/vga.c -o drivers/vga.o
	$(CC) $(CFLAGS) -c drivers/keyboard.c -o drivers/keyboard.o
	$(CC) $(CFLAGS) -Wl,--build-id=none -T kernel/linker.ld -o kernel.elf boot/multiboot.o kernel/gdt_flush.o kernel/isr_asm.o kernel/kernel.o kernel/shell.o kernel/gdt.o kernel/idt.o kernel/isr.o drivers/vga.o drivers/keyboard.o

myos.iso: kernel.elf
	cp kernel.elf iso/boot/
	grub-mkrescue -o myos.iso iso/

run: myos.iso
	qemu-system-i386 -cdrom myos.iso

clean:
	rm -f boot/multiboot.o kernel/gdt_flush.o kernel/isr_asm.o kernel/kernel.o kernel/shell.o kernel/gdt.o kernel/idt.o kernel/isr.o drivers/vga.o drivers/keyboard.o kernel.elf myos.iso iso/boot/kernel.elf

.PHONY: all run clean
CC = gcc
AS = nasm
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -nodefaultlibs

all: myos.iso

kernel.elf: boot/multiboot.asm kernel/kernel.c drivers/vga.c
	$(AS) -f elf32 boot/multiboot.asm -o boot/multiboot.o
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	$(CC) $(CFLAGS) -c drivers/vga.c -o drivers/vga.o
	$(CC) $(CFLAGS) -T kernel/linker.ld -o kernel.elf boot/multiboot.o kernel/kernel.o drivers/vga.o

myos.iso: kernel.elf
	cp kernel.elf iso/boot/
	grub-mkrescue -o myos.iso iso/

run: myos.iso
	qemu-system-i386 -cdrom myos.iso

clean:
	rm -f boot/multiboot.o kernel/kernel.o drivers/vga.o kernel.elf myos.iso iso/boot/kernel.elf

.PHONY: all run clean
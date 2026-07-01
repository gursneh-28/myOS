CC = gcc
AS = nasm
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -nodefaultlibs -no-pie -fno-pic

all: myos.iso

kernel.elf: boot/multiboot.asm kernel/kernel.c kernel/shell.c kernel/pmm.c kernel/heap.c kernel/paging.c kernel/task.c kernel/gdt.c kernel/idt.c kernel/isr.c kernel/gdt_flush.asm kernel/isr.asm drivers/vga.c drivers/keyboard.c drivers/timer.c
	$(AS) -f elf32 boot/multiboot.asm -o boot/multiboot.o
	$(AS) -f elf32 kernel/gdt_flush.asm -o kernel/gdt_flush.o
	$(AS) -f elf32 kernel/isr.asm -o kernel/isr_asm.o
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	$(CC) $(CFLAGS) -c kernel/shell.c -o kernel/shell.o
	$(CC) $(CFLAGS) -c kernel/pmm.c -o kernel/pmm.o
	$(CC) $(CFLAGS) -c kernel/heap.c -o kernel/heap.o
	$(CC) $(CFLAGS) -c kernel/paging.c -o kernel/paging.o
	$(CC) $(CFLAGS) -c kernel/task.c -o kernel/task.o
	$(CC) $(CFLAGS) -c kernel/gdt.c -o kernel/gdt.o
	$(CC) $(CFLAGS) -c kernel/idt.c -o kernel/idt.o
	$(CC) $(CFLAGS) -c kernel/isr.c -o kernel/isr.o
	$(CC) $(CFLAGS) -c drivers/vga.c -o drivers/vga.o
	$(CC) $(CFLAGS) -c drivers/keyboard.c -o drivers/keyboard.o
	$(CC) $(CFLAGS) -c drivers/timer.c -o drivers/timer.o
	$(CC) $(CFLAGS) -Wl,--build-id=none -T kernel/linker.ld -o kernel.elf boot/multiboot.o kernel/gdt_flush.o kernel/isr_asm.o kernel/kernel.o kernel/shell.o kernel/pmm.o kernel/heap.o kernel/paging.o kernel/task.o kernel/gdt.o kernel/idt.o kernel/isr.o drivers/vga.o drivers/keyboard.o drivers/timer.o

myos.iso: kernel.elf
	cp kernel.elf iso/boot/
	grub-mkrescue -o myos.iso iso/

run: myos.iso
	qemu-system-i386 -cdrom myos.iso

clean:
	rm -f boot/multiboot.o kernel/gdt_flush.o kernel/isr_asm.o kernel/kernel.o kernel/shell.o kernel/pmm.o kernel/heap.o kernel/paging.o kernel/task.o kernel/gdt.o kernel/idt.o kernel/isr.o drivers/vga.o drivers/keyboard.o drivers/timer.o kernel.elf myos.iso iso/boot/kernel.elf

.PHONY: all run clean
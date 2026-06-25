# myOS

A custom x86 operating system built from scratch in C and x86 Assembly.
Runs on QEMU virtual machine.

## Structure
- 'boot/' - Bootloader (NASM assembly)
- 'kernel/' - Kernel core (C)
- 'drivers/' - Hardware drivers
- 'libc/' - Mini C standard library
- 'iso/' - Bootable ISO structure

## Tools
- GCC i686-elf cross compiler
- NASM assembler
- QEMU emulator
- GRUB bootloader

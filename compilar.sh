#!/bin/bash

# assembly principal
rm *.o
nasm -f elf -l loader.lst -o loader.o loader.s
nasm -f elf -l gdt.lst -o gdt.o gdt.asm
nasm -f elf -l print_protegido.lst -o print_protegido.o print_protegido.asm

nasm -f elf -l idt.lst -o idt.o idt.asm


# gcc -m32 -g -ggdb -Wall -Werror -O0 -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding -c -o gdt.o gdt.c

gcc  -fverbose-asm -masm=intel -g -o kernel.o -c kernel.c -Wall -Wextra -Werror  -nostdlib -nostartfiles -nodefaultlibs

ld -T linker.ld -o kernel.elf idt.o loader.o kernel.o gdt.o print_protegido.o 

mcopy -i diskette.img kernel.elf ::/BOOT
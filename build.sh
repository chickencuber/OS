#!/bin/bash
i686-elf-gcc -ffreestanding -m32 -g -c "./src/kernel.c" -o "./bin/kernel.o"
nasm ./src/kernel_entry.asm -f elf -o ./bin/kernel_entry.o
i686-elf-ld -o bin/kernel.bin -Ttext 0x1000 bin/kernel_entry.o bin/kernel.o --oformat binary
nasm ./src/boot.asm -f bin -o ./bin/boot.bin
cat ./bin/boot.bin ./bin/kernel.bin > ./bin/os.bin

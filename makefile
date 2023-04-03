# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Disk
DISK_NAME      = storage

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2023.iso

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -ffreestanding -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386
ISOFLAGS	  = -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table

run: all
	@qemu-system-i386 -s -S -drive file=bin/storage.bin,format=raw,if=ide,index=0,media=disk -cdrom bin/os2023.iso
	
all: build

build: iso

clean:
	rm -rf *.o *.iso $(OUTPUT_FOLDER)/kernel

kernel:
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/portio/portio.c -o $(OUTPUT_FOLDER)/portio.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/std/stdmem.c -o $(OUTPUT_FOLDER)/stdmem.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/gdt/gdt.c -o $(OUTPUT_FOLDER)/gdt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/framebuffer/framebuffer.c -o $(OUTPUT_FOLDER)/framebuffer.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/interrupt.c -o $(OUTPUT_FOLDER)/interrupt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/idt.c -o $(OUTPUT_FOLDER)/idt.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/interrupt/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/keyboard/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/disk.c -o $(OUTPUT_FOLDER)/disk.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/cmos.c -o $(OUTPUT_FOLDER)/cmos.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/fat32.c -o $(OUTPUT_FOLDER)/fat32.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/paging/paging.c -o $(OUTPUT_FOLDER)/paging.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel/kernel_loader.s -o $(OUTPUT_FOLDER)/kernel_loader.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/kernel/kernel.c -o $(OUTPUT_FOLDER)/kernel.o
	@$(LIN) $(LFLAGS) bin/*.o -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f *.o

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	@cd bin && genisoimage -R -b boot/grub/grub1 $(ISOFLAGS) -o $(ISO_NAME) iso

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M
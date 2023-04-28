#include "portio/portio.h"
#include "std/stdtype.h"
#include "std/stdmem.h"
#include "gdt/gdt.h"
#include "framebuffer/framebuffer.h"
#include "kernel_loader.h"
#include "interrupt/interrupt.h"
#include "interrupt/idt.h"
#include "filesystem/fat32.h"
#include "keyboard/keyboard.h"
#include "paging/paging.h"
#include "kernel_loader.h"

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Allocate first 4 MiB virtual memory
    allocate_single_user_page_frame((uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    
    read(request);
    struct ClusterBuffer cbuf[2];
    memcpy(cbuf, "Nandemonai to kuchi wo tsugunda\nHonto wa chotto ashi wo tometaku\n", 67);
    request.buf = cbuf;
    memcpy(&request.name, "ikana", 5);
    memcpy(&request.ext, "txt", 3);
    request.buffer_size = CLUSTER_SIZE;
    write(request);
    memcpy(cbuf, "Ini dari file lain\n", 20);
    memcpy(&request.name, "laina", 5);
    write(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);
    
    while (TRUE);
}
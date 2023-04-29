#include "portio/portio.h"
#include "std/stdtype.h"
#include "std/stdmem.h"
#include "std/string.h"
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
    memcpy(cbuf, "Yami wo haratte yami wo haratte\nYoru no tobari ga oritara aizu da\nAitai shite mawaru kanjousen\nZaregoto nado wa hakisute ike to\n", 132);
    request.buf = cbuf;
    clear(&request.name, 8);
    clear(&request.ext, 3);
    memcpy(&request.name, "kaikai", 6);
    memcpy(&request.ext, "txt", 3);
    request.buffer_size = CLUSTER_SIZE;
    write(request);
    clear(&request.name, 8);
    memcpy(cbuf, "Ini dari file lain\n", 20);
    memcpy(&request.name, "lain", 4);
    write(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);
    
    while (TRUE);
}
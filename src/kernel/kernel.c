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

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();

    struct ClusterBuffer cbuf[5];
    for (uint32_t i = 0; i < 5; i++)
        for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
            cbuf[i].buf[j] = i + 'a';

    struct FAT32DriverRequest request = {
        .buf                   = cbuf,
        .name                  = "ikanaide",
        .ext                   = "uwu",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0,
    } ;

    struct ClusterBuffer cbuf2[5];
    struct FAT32DriverRequest request2 = {
        .buf                   = cbuf2,
        .name                  = "daijoubu",
        .ext                   = "uwu",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0,
    } ;

    struct ClusterBuffer cbuf3[5];
    struct FAT32DriverRequest request3 = {
        .buf                   = cbuf3,
        .name                  = "hahahaha",
        .ext                   = "uwu",
        .parent_cluster_number = 4,
        .buffer_size           = 0,
    } ;

    write(request);  // Create folder "ikanaide"
    memcpy(request.name, "kano1\0\0\0", 8);
    write(request);  // Create folder "kano1"
    memcpy(request.name, "ikanaide", 8);
    delete(request); // Delete first folder, thus creating hole in FS

    memcpy(request.name, "daijoubu", 8);
    request.buffer_size = 5*CLUSTER_SIZE;
    write(request);  // Create fragmented file "daijoubu"

    memcpy(request.name, "hahahaha", 8);
    request.parent_cluster_number = 4;
    write(request);  // Create fragmented file "hahahaha" inside kano1 folder

    struct ClusterBuffer readcbuf;
    read_clusters(&readcbuf, ROOT_CLUSTER_NUMBER+1, 1); 
    // If read properly, readcbuf should filled with 'a'

    request2.buffer_size = CLUSTER_SIZE;
    read(request2);   // Failed read due not enough buffer size
    request2.buffer_size = 5*CLUSTER_SIZE;
    read(request2);   // Success read on file "daijoubu"
    read_directory(request2); // Failed directory read (not a folder)
    memcpy(request2.name, "kano1\0\0\0", 8);
    read_directory(request2); // Success directory read kano1
    request3.buffer_size = 5*CLUSTER_SIZE;
    read(request3);   // Success read on file "daijoubu"

    while (TRUE)
        keyboard_state_activate();
}
#include "../portio/portio.h"
#include "../std/stdtype.h"
#include "../std/stdmem.h"
#include "../gdt/gdt.h"
#include "../framebuffer/framebuffer.h"
#include "kernel_loader.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/idt.h"
#include "../filesystem/fat32.h"

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    if (is_empty_storage()) {
      framebuffer_write(3,4,'C',0,255);
    }
    initialize_filesystem_fat32();
    if (is_empty_storage()) {
      framebuffer_write(3,5,'A',0,255);
    }
    while (TRUE); 
      // keyboard_state_activate();
}
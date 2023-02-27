#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    framebuffer_clear();
    framebuffer_write(11, 35,  'A', 0, 0xF);
    framebuffer_write(11, 36,  'p', 0, 0xF);
    framebuffer_write(11, 37, 'a', 0, 0xF);
    framebuffer_write(11, 38, 'G', 0, 0xF);
    framebuffer_write(11, 39, 'a', 0, 0xF);
    framebuffer_write(11, 40, 'K', 0, 0xF);
    framebuffer_write(11, 41, 'e', 0, 0xF);
    framebuffer_write(11, 42, 'O', 0, 0xF);
    framebuffer_write(11, 43, 'S', 0, 0xF);
    framebuffer_set_cursor(11, 39);
    while (TRUE);
}
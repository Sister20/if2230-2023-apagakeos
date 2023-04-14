#include "std/stdtype.h"
#include "std/string.h"
#include "filesystem/fat32.h"

// Color declarations
#define BIOS_LIGHT_GREEN 0b1010
#define BIOS_GREY        0b0111
#define BIOS_LIGHT_BLUE  0b1001 

void interrupt (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void put (char* buf, uint8_t color) {
    const char *s = buf;
    while (*s++);
    interrupt (5, (uint32_t) buf, s - buf - 1, color);
    __asm__("int $0x30");
}

int main(void) {
    char buf[20];
    while (TRUE) {
        put("ApaGaKeOS@OS-IF2230", BIOS_LIGHT_GREEN);
        put(":", BIOS_GREY);
        put("/", BIOS_LIGHT_BLUE);
        put("$", BIOS_GREY);
        interrupt (5, (uint32_t) buf, 20, 0x0);
    }

    return 0;
}
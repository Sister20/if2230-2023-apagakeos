#include "std/stdtype.h"
#include "filesystem/fat32.h"
#include "std/string.h"

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
    size_t i = 0;
    while (buf[i] != '\0')
        i++;
    interrupt (5, (uint32_t) buf, i, color);
}

int main(void) {
    char buffer[300];
    char* init[4] = {"ApaGaKeOS@OS-IF2230", ":", "/", "$ "};
    uint8_t color[4] = {BIOS_LIGHT_GREEN, BIOS_GREY, BIOS_LIGHT_BLUE, BIOS_GREY};
    while (TRUE) {
        for (int i = 0; i < 4; i++) {
            put(init[i], color[i]);
        }
        interrupt (4, (uint32_t) buffer, 300, 0x0);
    }

    return 0;
}
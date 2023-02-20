#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"

// void framebuffer_set_cursor(uint8_t r, uint8_t c) {
//     // TODO : Implement
// }

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *)0xB8000 + (row * 80 + col) ;
    *where = c | (attrib << 8);
}

void framebuffer_clear(void) {
    // Implementasi dari : https://wiki.osdev.org/Text_UI bagian Scrolling
    for (int i=0; i<25; i++) {
        for (int j=0; j < 80; j++) {
            framebuffer_write(i,j,' ', 0xF, 0);
        }
    }
}

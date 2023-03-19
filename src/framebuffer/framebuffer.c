#include "framebuffer/framebuffer.h"
#include "std/stdtype.h"
#include "std/stdmem.h"
#include "portio/portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    // Initialized the cursor position byte
    uint16_t position = r*80 + c;

    // Give the cursor position to hardware port
    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint8_t) (position & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E);
    out(CURSOR_PORT_DATA, (uint8_t) ((position >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    // Merge the colour byte and char into a complete 2 byte 
    uint16_t colourBit = (bg << 4) | fg ;
    uint16_t fullBit = (colourBit << 8) | c ;

    // Assign the character 2 byte into the memory
    volatile uint16_t* where = (volatile uint16_t*) MEMORY_FRAMEBUFFER + (row*80 + col) ;
    *where = fullBit;
}

void framebuffer_clear(void) {
    // Set all character 2 byte in the 80x25 framebuffer into black blank
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            framebuffer_write(i, j, 0x00, 0x7, 0);
        }
    }
}

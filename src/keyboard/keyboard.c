#include "keyboard.h"
#include "lib-header/portio.h"
#include "lib-header/framebuffer.h"
#include "lib-header/stdmem.h"

struct KeyboardDriverState keyboard_state;

static int keyboard_buffer_write_pos = 0;
static int keyboard_buffer_read_pos = 0;

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void activate_keyboard_interrupt(void) {
  out(PIC1_DATA, PIC_DISABLE_ALL_MASK ^ (1 << IRQ_KEYBOARD));
  out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void deactivate_keyboard_interrupt(void) {
  out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
  out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void) {
    if (!keyboard_state.keyboard_input_on) {
        activate_keyboard_interrupt();
        keyboard_state.keyboard_input_on = TRUE;
    }
}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void) {
    if (keyboard_state.keyboard_input_on) {
        deactivate_keyboard_interrupt();
        keyboard_state.keyboard_input_on = FALSE;
    }
}

// Get keyboard buffer values - @param buf Pointer to char buffer, recommended size at least KEYBOARD_BUFFER_SIZE
void get_keyboard_buffer(char *buf) {
    int i=0;
    while (keyboard_buffer_read_pos != keyboard_buffer_write_pos) {
        buf[i] = keyboard_state.keyboard_buffer[keyboard_buffer_read_pos];
        keyboard_buffer_read_pos = (keyboard_buffer_read_pos + 1) % KEYBOARD_BUFFER_SIZE;
        i++;
    }
    buf[i] = '\0';
}

// Check whether keyboard ISR is active or not - @return Equal with keyboard_input_on value
bool is_keyboard_blocking(void) {
    return keyboard_state.keyboard_input_on;
}

void keyboard_isr(void) {
    if (!keyboard_state.keyboard_input_on)
        keyboard_state.buffer_index = 0;
    else {
        uint8_t  scancode    = in(KEYBOARD_DATA_PORT);
        char     mapped_char = keyboard_scancode_1_to_ascii_map[scancode];
        if (scancode == '\b' && keyboard_state.buffer_index > 0) {
            keyboard_state.buffer_index--;
            screen_putc('\b');
            screen_putc(' ');
            screen_putc('\b');
        } else if (mapped_char != 0 && keyboard_state.keyboard_buffer < KEYBOARD_BUFFER_SIZE - 1) {
            keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = mapped_char;
            keyboard_state.buffer_index++;
            screen_putc(mapped_char);
        }

        if (mapped_char == '\n') {
            keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = '\0';
            keyboard_state.buffer_index = 0;
            screen_putc('\n');
        }
    }
    pic_ack(IRQ_KEYBOARD);
}
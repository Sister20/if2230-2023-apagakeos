#include "std/stdtype.h"
#include "idt.h"

/**
 * interrupt_descriptor_table, predefined IDT.
 * Initial IDTGate already set properly according to IDT definition
 */
struct InterruptDescriptorTable interrupt_descriptor_table;

/**
 * _idt_idtr, predefined system IDTR. 
 * IDT pointed by this variable is already set to point global_descriptor_table above.
*/
struct IDTR _idt_idtr = {
    .size = sizeof(interrupt_descriptor_table) - 1,
    .address = &interrupt_descriptor_table
};

void set_interrupt_gate(uint8_t int_vector, void *handler_address, uint16_t gdt_seg_selector, uint8_t privilege) {
    struct IDTGate *idt_int_gate = &interrupt_descriptor_table.table[int_vector];
    idt_int_gate->offset_low  = (uint32_t)handler_address & 0xFFFF;
    idt_int_gate->segment     = gdt_seg_selector;

    // Target system 32-bit and flag this as valid interrupt gate
    idt_int_gate->_r_bit_1    = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->_r_bit_2    = INTERRUPT_GATE_R_BIT_2;
    idt_int_gate->gate_32     = 1;
    idt_int_gate->_r_bit_3    = INTERRUPT_GATE_R_BIT_3;
    idt_int_gate->dpl_bit     = privilege;
    idt_int_gate->valid_bit   = 1;
    idt_int_gate->offset_high = (uint32_t)handler_address >> 16;
}

extern void* isr_stub_table[];

void initialize_idt(void) {
    for (uint8_t i = 0; i < ISR_STUB_TABLE_LIMIT; i++) {
        if (i >= 0x30 && i <= 0x3F) {
            set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0x3);
        } else {
            set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0);
        }
    }

    __asm__ volatile("lidt %0" : : "m"(_idt_idtr)); // load the new IDT
    __asm__ volatile("sti");  // set the interrupt flag
}
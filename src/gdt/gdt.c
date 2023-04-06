#include "std/stdtype.h"
#include "gdt/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to GDT definition in Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // Null Descriptor
            .segment_low = 0x0000,
            .base_low = 0x0000,

            .base_mid = 0x00,
            .type_bit = 0x00,
            .non_system = 0x00,
            .dpl_bit = 0x00,
            .present = 0x00,
            .segment_high = 0x00,
            .available_bit = 0x00,
            .code_in_64bit = 0x00,
            .db_flag = 0x00,
            .granularity = 0x00,
            .base_high = 0x00,
        },
        {
            // Kernel code segment
            .segment_low = 0xFFFF,
            .base_low = 0x0000,

            .base_mid = 0x00,
            .type_bit = 0xA,
            .non_system = 0x01,
            .dpl_bit = 0x00,
            .present = 0x01,
            .segment_high = 0xF,
            .available_bit = 0x00,
            .code_in_64bit = 0x00,
            .db_flag = 0x01,
            .granularity = 0x01,
            .base_high = 0x00,
        },
        {
            // Kernel data segment
            .segment_low = 0xFFFF,
            .base_low = 0x0000,

            .base_mid = 0x00,
            .type_bit = 0x2,
            .non_system = 0x01,
            .dpl_bit = 0x00,
            .present = 0x01,
            .segment_high = 0xF,
            .available_bit = 0x00,
            .code_in_64bit = 0x00,
            .db_flag = 0x01,
            .granularity = 0x01,
            .base_high = 0x00,
        },
        {
            // User code segment
            .segment_low = 0xFFFF,
            .base_low = 0x0000,

            .base_mid = 0x00,
            .type_bit = 0xA,
            .non_system = 0x01,
            .dpl_bit = 0x03,
            .present = 0x01,
            .segment_high = 0xF,
            .available_bit = 0x00,
            .code_in_64bit = 0x00,
            .db_flag = 0x01,
            .granularity = 0x01,
            .base_high = 0x00,
        },
        {
            // User data segment
            .segment_low = 0xFFFF,
            .base_low = 0x0000,

            .base_mid = 0x00,
            .type_bit = 0x2,
            .non_system = 0x01,
            .dpl_bit = 0x03,
            .present = 0x01,
            .segment_high = 0xF,
            .available_bit = 0x00,
            .code_in_64bit = 0x00,
            .db_flag = 0x01,
            .granularity = 0x01,
            .base_high = 0x00,
        },
        {
            .segment_high      = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .dpl_bit           = 0,    // DPL
            .present           = 1,    // P bit
            .db_flag           = 1,    // D/B bit
            .code_in_64bit     = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    .size = sizeof(global_descriptor_table) - 1,
    .address = &global_descriptor_table
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}

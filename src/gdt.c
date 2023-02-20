#include "lib-header/stdtype.h"
#include "lib-header/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to GDT definition in Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // Null Descriptor
            // Base = 0x00
            // Limit = 0x00
            // Access Byte = 0x0
            // Flags = 0x0
        },
        {
            // Kernel code segment
            // Base = 0x00
            // Limit = 0xFFFFF
            // Access Byte = 0x9A
            // Flags = 0xCF
        },
        {
            // Kernel data segment
            // Base = 0x00
            // Limit = 0xFFFFF
            // Access Byte = 0x92
            // Flags = 0xCF
        }
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
};
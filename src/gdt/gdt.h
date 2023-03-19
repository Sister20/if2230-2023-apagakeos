#ifndef _GDT_H
#define _GDT_H

#include "../std/stdtype.h"

#define GDT_MAX_ENTRY_COUNT 32

extern struct GDTR _gdt_gdtr;

/**
 * Segment Descriptor storing system segment information.
 * Struct defined exactly as Intel Manual Segment Descriptor definition (Figure 3-8 Segment Descriptor).
 * Manual can be downloaded at www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html/ 
 *
 * @param segment_low   16-bit lower-bit segment limit
 * @param base_low      16-bit lower-bit base address
 * @param base_mid      8-bit middle-bit base address
 * @param type_bit      4-bit contain type flags
 * @param non_system    1-bit contain system (S)
 * @param dpl_bit       2-bit contain descriptor privilege level (DPL)
 * @param present       1-bit contain segment existence in memory (P)
 * @param segment_high  4-bit higher-bit segment limit
 * @param available_bit 1-bit contain free-use bit (AVL)
 * @param code_in_64bit 1-bit contain 64-bit code existence (L)
 * @param db_flag       1-bit contain operation size (D/B)
 * @param granularity   1-bit contain scaling of segment limit field (G)
 * @param base_high     8-bit higher base address
 */
struct SegmentDescriptor {
    // First 32-bit
    uint16_t segment_low;
    uint16_t base_low;

    // Last 32-bit (Bit 32 to 63)
    uint8_t base_mid;
    uint8_t type_bit        : 4;
    uint8_t non_system      : 1;
    uint8_t dpl_bit         : 2;
    uint8_t present         : 1;
    uint8_t segment_high    : 4;
    uint8_t available_bit   : 1;
    uint8_t code_in_64bit   : 1;
    uint8_t db_flag         : 1;
    uint8_t granularity     : 1;
    uint8_t base_high;

} __attribute__((packed));

/**
 * Global Descriptor Table containing list of segment descriptor. One GDT already defined in memory.c.
 * More details at https://wiki.osdev.org/GDT_Tutorial
 * @param table Fixed-width array of SegmentDescriptor with size GDT_MAX_ENTRY_COUNT
 */
struct GlobalDescriptorTable {
    struct SegmentDescriptor table[GDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

/**
 * GDTR, carrying information where's the GDT located and GDT size.
 * Global kernel variable defined at memory.c.
 * 
 * @param size    Global Descriptor Table size, use sizeof operator
 * @param address GDT address, GDT should already defined properly
 */
struct GDTR {
    uint16_t                     size;
    struct GlobalDescriptorTable *address;
} __attribute__((packed));

#endif
#include "gdt.h"

#include <debug.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags_limit_high;
    uint8_t base_high;
} __attribute__((packed)) GDT_Entry;

typedef struct {
    uint16_t limit;
    GDT_Entry *entries;
} __attribute__((packed)) GDT_Descriptor;

typedef enum {
    GDT_ACCESS_CODE_READABLE    = 0x02,
    GDT_ACCESS_DATA_WRITEABLE   = 0x02,

    GDT_ACCESS_CODE_CONFORMING          = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL    = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN      = 0x04,

    GDT_ACCESS_DATA_SEGMENT             = 0x10,
    GDT_ACCESS_CODE_SEGMENT             = 0x18,
    
    GDT_ACCESS_DESCRIPTOR_TSS           = 0x00,

    GDT_ACCESS_RING0                    = 0x00,
    GDT_ACCESS_RING1                    = 0x20,
    GDT_ACCESS_RING2                    = 0x40,
    GDT_ACCESS_RING3                    = 0x60,

    GDT_ACCESS_PRESENT                  = 0x80,
} GDT_ACCESS;

typedef enum {
    GDT_FLAG_64BIT          = 0x20,
    GDT_FLAG_32BIT          = 0x40,
    GDT_FLAG_16BIT          = 0x00,
    GDT_FLAG_GRANULARITY_1B = 0x00,
    GDT_FLAG_GRANULARITY_4K = 0x80
} GDT_FLAGS;

#define GDT_ENTRY_COUNT 3

__attribute__((aligned(0x10)))
GDT_Entry gdt[GDT_ENTRY_COUNT];

void encode_gdt_entry(
    int entry, uint32_t base, uint32_t limit, 
    uint8_t access, uint8_t flags
) {
    if (entry >= GDT_ENTRY_COUNT)
        panic("GDT", "Invalid GDT entry %d (Max %d)", entry, GDT_ENTRY_COUNT-1);

    gdt[entry].flags_limit_high = 0;
    gdt[entry].limit_low = limit & 0xFFFF;
    gdt[entry].flags_limit_high |= (limit >> 16) & 0xF;

    gdt[entry].base_low = base & 0xFFFF;
    gdt[entry].base_mid = (base >> 16) & 0xFF;
    gdt[entry].base_high = (base >> 24) & 0xFF;

    gdt[entry].access = access;
    gdt[entry].flags_limit_high |= flags;
}

GDT_Descriptor gdt_descriptor = { sizeof(gdt) - 1, gdt };

void __attribute__((cdecl)) i686_gdt_load(GDT_Descriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);

void gdt_initialize() {
    // Zero all entries
    for (int i = 0; i < GDT_ENTRY_COUNT; i++) {
        encode_gdt_entry(i, 0, 0, 0, 0);
    }

    // Setup code segment
    encode_gdt_entry(
        1, 0, 0xFFFFF, 
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT |
        GDT_ACCESS_CODE_READABLE | 1,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K
    );

    // Setup data segment
    encode_gdt_entry(
        2, 0, 0xFFFFF, 
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT |
        GDT_ACCESS_DATA_WRITEABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K
    );

    i686_gdt_load(&gdt_descriptor, GDT_CODE_SEGMENT, GDT_DATA_SEGMENT);
}
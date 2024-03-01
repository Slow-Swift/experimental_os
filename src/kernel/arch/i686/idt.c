#include "idt.h"

#include <stdint.h>
#include <defs.h>
#include "gdt.h"

typedef struct {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
} __attribute__((packed)) IDT_Entry;

typedef struct {
    uint16_t size;
    IDT_Entry* offset;
} IDT_Descriptor;

__attribute__((aligned(0x10)))
static IDT_Entry idt[256];
static IDT_Descriptor idt_descriptor = { sizeof(idt) - 1, idt };

void ASMCALL idt_load(IDT_Descriptor* descriptor);

void idt_set_gate(
    int interrupt, void* base, uint16_t segment_descriptor, uint8_t flags
) {
    idt[interrupt].offset_low = ((uint32_t)base) & 0xFFFF;
    idt[interrupt].offset_high = ((uint32_t)base >> 16) & 0xFFFF;
    idt[interrupt].segment_selector = segment_descriptor;
    idt[interrupt].reserved = 0;
    idt[interrupt].flags = flags;
}

void idt_enable_gate(int interrupt) 
{
    idt[interrupt].flags |= IDT_FLAG_PRESENT;
}

void idt_disable_gate(int interrupt) 
{
    idt[interrupt].flags &= ~IDT_FLAG_PRESENT;
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    IDT_Entry* descriptor = &idt[vector];
 
    descriptor->offset_low        = (uint32_t)isr & 0xFFFF;
    descriptor->segment_selector      = GDT_CODE_SEGMENT; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->flags     = flags;
    descriptor->offset_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

extern void* isr_stub_table[];

void idt_initialize() {
    idt_descriptor.offset = &idt[0];
    idt_descriptor.size = (uint16_t)sizeof(IDT_Entry) * 256 - 1;
 
    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }
    idt_load(&idt_descriptor);
}
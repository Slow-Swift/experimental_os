#include "idt.h"

#define FLAG_SET(x, flag) (x) |= (flag)
#define FLAG_UNSET(x, flag) (x) &= ~(flag)

typedef struct
{
    uint16_t base_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) IDT_Entry;

typedef struct {
    uint16_t limit;
    IDT_Entry* base;
} __attribute__((packed)) IDT_Descriptor;

IDT_Entry idt[256];
IDT_Descriptor idt_descriptor = { sizeof(idt) - 1, idt };

void __attribute__((cdecl)) idt_load(IDT_Descriptor* idt_descriptor);

void idt_set_gate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags) {
    idt[interrupt].base_low = ((uint32_t)base) & 0xFFFF;
    idt[interrupt].segment_selector = segmentDescriptor;
    idt[interrupt].reserved = 0;
    idt[interrupt].flags = flags;
    idt[interrupt].base_high = ((uint32_t)base >> 16) & 0xFFFF;
}

void idt_enable_gate(int interrupt) {
    idt[interrupt].flags |= IDT_FLAG_PRESENT;
}

void idt_disable_gate(int interrupt) {
    idt[interrupt].flags &= ~IDT_FLAG_PRESENT;
}

void idt_initialize() {
    idt_load(&idt_descriptor);
}

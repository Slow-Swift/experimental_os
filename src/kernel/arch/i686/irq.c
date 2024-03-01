#include "irq.h"

#include "io.h"
#include "isr.h"
#include <stddef.h>
#include <stdio.h>
#include "pic.h"

#define PIC_REMAP_OFFSET 0x20

IRQ_Handler irq_handlers[16];

static void irq_handler(Registers *regs) {
    int irq = regs->interrupt - PIC_REMAP_OFFSET;

    if (irq_handlers[irq] != NULL) {
        irq_handlers[irq](regs);
    } else {
        printf("Unhandled IRQ %d\n", irq);
    }

    pic_send_EOI(irq);
}

void irq_initialize() {
    pic_configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    for (int i = 0; i < 16; i++)
        isr_register_handler(PIC_REMAP_OFFSET + i, irq_handler);

    enable_interrupts();
}

void register_handler(int irq, IRQ_Handler handler) {
    irq_handlers[irq] = handler;
    pic_unmask(irq);
}
#include "irq.h"
#include "pic.h"
#include "i8259.h"
#include "io.h"
#include <stdio.h>
#include <stddef.h>

#define SIZE(array) (sizeof(array) / sizeof(array[0]))

#define PIC_REMAP_OFFSET    0x20

IRQHandler irq_handlers[16];
static const PIC_Driver* driver = NULL;

void irq_handler(Registers *regs) {
    int irq = regs->interrupt - PIC_REMAP_OFFSET;

    if (irq_handlers[irq] != NULL) {
        irq_handlers[irq](regs);
    } 
    else {
        printf("Unhandled IRQ %d\n", irq);
    }

    driver->send_eoi(irq);
}

void irq_initialize() {
    const PIC_Driver* drivers[] = {
        i8259_get_driver(),
    };

    for (int i=0; i<SIZE(drivers); i++) {
        if (drivers[i]->probe()) {
            driver = drivers[i];
        }
    }

    if (driver == NULL) {
        printf("Warning: No PIC found!");
        return;
    }

    printf("Using PIC Driver: %s\n", driver->name);
    driver->initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    for (int i=0; i<16; i++)
        isr_register_handler(PIC_REMAP_OFFSET + i, irq_handler);
}

void irq_register_handler(int irq, IRQHandler handler) {
    irq_handlers[irq] = handler;
    driver->unmask(irq);
}

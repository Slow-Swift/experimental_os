#include "hal.h"

#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/irq.h>
#include <arch/i686/isr.h>

void hal_initialize() {
    gdt_initialize();
    idt_initialize();
    isr_initialize();
    irq_initialize();
}
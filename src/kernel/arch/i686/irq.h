#pragma once

#include "isr.h"

typedef void (*IRQ_Handler)(Registers* regs);

void irq_initialize();
void irq_register_handler(int irq, IRQ_Handler handler);
#pragma once

#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void irq_initialize();
void irq_register_handler(int irq, IRQHandler handler);

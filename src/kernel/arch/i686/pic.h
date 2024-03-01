#pragma once

#include <stdbool.h>
#include <stdint.h>

void pic_configure(int offset1, int offset2, bool auto_eoi);
void pic_mask(int irq_line);
void pic_unmask(int irq_line);
uint16_t pic_get_irr();
uint16_t pic_get_isr();
void pic_send_EOI(int irq_line);
void pic_disable();
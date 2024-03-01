#pragma once

#include <stdint.h>
#include <defs.h>

void ASMCALL disable_interrupts();
void ASMCALL enable_interrupts();
void ASMCALL out_byte(uint16_t port, uint8_t value);
uint8_t ASMCALL in_byte(uint16_t port);
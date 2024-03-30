#pragma once

#include <stdint.h>
#include <defs.h>

void ASMCALL disable_interrupts();
void ASMCALL enable_interrupts();
void ASMCALL out_byte(uint16_t port, uint8_t value);
uint8_t ASMCALL in_byte(uint16_t port);
void ASMCALL out_word(uint16_t port, uint32_t value);
uint32_t ASMCALL in_word(uint16_t port);
void ASMCALL out_double(uint16_t port, uint32_t value);
uint32_t ASMCALL in_double(uint16_t port);

void ASMCALL io_wait();
void ASMCALL panic_stop();
void ASMCALL halt();
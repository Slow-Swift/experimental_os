#pragma once

#include <stdint.h>
#include <defs.h>

void ASMCALL disable_interrupts();
void ASMCALL enable_interrupts();
void ASMCALL out_byte(uint16_t port, uint8_t value);
uint8_t ASMCALL in_byte(uint16_t port);

#define i686_outb(port, value) out_byte(port, value)
#define i686_inb(port) in_byte(port)
#define i686_EnableInterrupts() enable_interrupts()

void i686_iowait();
void __attribute__((cdecl)) i686_Panic();

#pragma once

#include <stdint.h>
#include <defs.h>

void ASMCALL out_byte(uint16_t port, uint8_t value);
uint8_t ASMCALL in_byte(uint16_t port);
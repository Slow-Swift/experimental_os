#pragma once
#include <stdint.h>
#include <stdarg.h>

void puts(const char* str);
void puthex(uint64_t value);
void printf(const char* fmt, ...);
void clear_screen();
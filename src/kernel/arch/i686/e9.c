#include "e9.h"
#include "io.h"

void e9_putc(char c) {
    out_byte(0xE9, c);
}
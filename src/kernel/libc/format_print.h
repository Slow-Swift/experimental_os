#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include "stdio.h"

typedef int (*char_printer)(int, FILE * restrict);

int format_print(
    const char *fmt, FILE * restrict stream, 
    char_printer printc, va_list arg
);
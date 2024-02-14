#include "stdio.h"

#define ASMCALL __attribute__((cdecl))

extern void _init();

void halt();

void ASMCALL Start() {
    _init();

    puts("Hello World from the Kernel!");
    
    halt();
}

void halt() {
    for(;;);
}
#include <arch/i686/vga_text.h>
#include "bootdata.h"
#include "debug.h"
#include "defs.h"
#include "memory.h"
#include <stdio.h>

extern void _init();

void halt();

void ASMCALL Start(BootData* boot_data) 
{
    _init();

    log_info("Main", "Kernel Started");

    vga_clear_screen();
    printf("Kernel Started\n");

    memory_initialize(boot_data);
    
    halt();
}

void halt() {
    printf("Halting");
    for(;;);
}
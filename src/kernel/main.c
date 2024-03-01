#include <arch/i686/apic.h>
#include <arch/i686/vga_text.h>
#include "bootdata.h"
#include "debug.h"
#include "defs.h"
#include <hal/hal.h>
#include <stdio.h>
#include <stdlib.h>
#include <arch/i686/io.h>

extern void _init();

void halt();

void ASMCALL Start(BootData* boot_data) 
{
    _init();

    log_info("Main", "Kernel Started");

    vga_clear_screen();
    printf("Kernel Started\n");

    memory_initialize(boot_data);

    hal_initialize();

    printf("Initialized HAL\n");

    // enable_interrupts();

    halt();
}

void halt() {
    printf("Halting");
    for(;;);
}
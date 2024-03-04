#include <arch/i686/apic.h>
#include <arch/i686/vga_text.h>
#include "bootdata.h"
#include "debug.h"
#include "defs.h"
#include <hal/hal.h>
#include <stdio.h>
#include <stdlib.h>
#include <arch/i686/io.h>
#include <arch/i686/irq.h>

extern void _init();

void halt();

void timer(Registers* regs) {
    printf(".");
}

void ASMCALL Start(BootData* boot_data) 
{
    _init();

    log_info("Main", "Kernel Started");

    vga_clear_screen();
    printf("Kernel Started\n");

    memory_initialize(boot_data);

    hal_initialize();

    printf("Initialized HAL\n");

    irq_register_handler(0, timer);

    halt();
}

void halt() {
    for(;;);
}
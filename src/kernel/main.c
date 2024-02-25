#include <arch/i686/vga_text.h>
#include "bootdata.h"
#include "debug.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

extern void _init();

void halt();

void ASMCALL Start(BootData* boot_data) 
{
    _init();

    log_info("Main", "Kernel Started");

    vga_clear_screen();
    printf("Kernel Started\n");

    memory_initialize(boot_data);

    void* m1 = malloc(16);
    void* m2 = malloc(48);
    void* m3 = malloc(10);
    free(m1);
    free(m2);
    m1 = malloc(48);
    m2 = malloc(16);
    free(m2);
    free(m1);
    free(m3);
    
    halt();
}

void halt() {
    printf("Halting");
    for(;;);
}
#include <arch/i686/apic.h>
#include <arch/i686/vga_text.h>
#include "bootdata.h"
#include "debug.h"
#include "defs.h"
#include <hal/hal.h>
#include "keyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <arch/i686/io.h>
#include <arch/i686/irq.h>
#include <arch/i686/io.h>
#include <stdlib.h>
#include "events.h"
#include "keyboard.h"
#include "bash.h"
#include "disk.h"

extern void _init();

void loop();

void timer(Registers* regs) {
    // printf(".");
}

void keypress_handler(void *args) {
    KeypressEvent *event = args;
    if (event->ascii != '\0' && !event->released)
        printf("%c", event->ascii);
    free(event);
}

void ASMCALL Start(BootData* boot_data) 
{
    _init();

    log_info("Main", "Kernel Started");

    vga_clear_screen();
    printf("Kernel Started\n");

    memory_initialize(boot_data);

    hal_initialize(boot_data);

    printf("Initialized HAL\n");

    irq_register_handler(0, timer);
    enable_interrupts();

    kbd_initialize();

    disk_initialize();

    bash_initialize();
    loop();
}

void loop() {
    for(;;) {
        while (get_event_count() > 0) {
            call_next_event();
        }
        halt();
    }
}
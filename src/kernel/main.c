#include "bootdata.h"
#include "debug.h"
#include "defs.h"
#include <libc/stdio.h>
#include <arch/i686/vga_text.h>

extern void _init();

void halt();

void ASMCALL Start(BootData* bootData) 
{
    _init();

    MemoryRegion* memoryRegion = (MemoryRegion*)bootData->MemoryMapAddr;

    vga_clear_screen();
    printf("Kernel Started\n");

    printf("Memory Regions: \n");
    printf("|             BASE |              END |           LENGTH |     TYPE |\n");
    for (int i=0; i<bootData->MemRegionCount; i++) {
        printf("| %16llx | %16llx | %16llx | %8lx |\n", memoryRegion[i].BaseAddress, memoryRegion[i].BaseAddress + memoryRegion[i].Length, memoryRegion[i].Length, memoryRegion[i].Type);
    }

    log_debug("MAIN", "Debug Message");
    log_info("MAIN", "Info Message");
    log_warn("MAIN", "Warning Message");
    log_error("MAIN", "Error Message");
    log_critical("MAIN", "Critical Message");
    
    halt();
}

void halt() {
    printf("Halting");
    for(;;);
}
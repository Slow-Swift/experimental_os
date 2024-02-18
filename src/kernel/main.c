#include "stdio.h"
#include "bootdata.h"

#define ASMCALL __attribute__((cdecl))

extern void _init();

void halt();

void ASMCALL Start(BootData* bootData) {
    _init();

    MemoryRegion* memoryRegion = (MemoryRegion*)bootData->MemoryMapAddr;

    clear_screen();
    printf("Hello World from the Kernel!\n");

    printf("Memory Regions: \n");
    printf("|             BASE |              END |           LENGTH |     TYPE |\n");
    for (int i=0; i<bootData->MemRegionCount; i++) {
        printf("| %16llx | %16llx | %16llx | %8lx |\n", memoryRegion[i].BaseAddress, memoryRegion[i].BaseAddress + memoryRegion[i].Length, memoryRegion[i].Length, memoryRegion[i].Type);
    }
    
    // printf("Memory Region Count: 0x%lx\n", bootData->MemRegionCount);
    // printf("  Region 1:\n");
    // printf("    Base:   %#llx\n", memoryRegion->BaseAddress);
    // printf("    Length: %#llx\n", memoryRegion->Length);
    // printf("    Type:   %#lx\n", memoryRegion->Type);
    halt();
}

void halt() {
    for(;;);
}
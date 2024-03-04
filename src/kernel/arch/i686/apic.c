#include "apic.h"

#include <cpuid.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "msr.h"

#define CPU_FEAT_EDX_APIC (1 << 9)
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP (1 << 8)
#define IA32_APIC_BASE_MSR_ENABLE (1 << 11)

static bool is_boot_cpu;
static uintptr_t apic_base = 0xFEE00000;

static bool is_apic_supported() {
    unsigned int eax, edx, unused;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
    return (edx & CPU_FEAT_EDX_APIC) != 0;
}

/**
 * Determine whether the APIC MSR indicates this is a boot cpu or not
*/
static bool determine_if_boot_cpu() {
    uint32_t eax, edx;
    msr_get(IA32_APIC_BASE_MSR, &eax, &edx);

    return eax & IA32_APIC_BASE_MSR_BSP;
}

static uintptr_t get_apic_base() {
    uint32_t eax, edx;
    msr_get(IA32_APIC_BASE_MSR, &eax, &edx);

    // If more than 32 bits can be used for an address then 
    // include the 4 high bits of the address
#ifdef __PHYSICAL_MEMORY_EXTENSION__
        return (eax & 0xfffff000) | ((edx & 0x0f) << 32);
#else
        return (eax & 0xfffff000);
#endif
}

static void set_apic_base(uintptr_t base) {
    uint32_t edx = 0;
    uint32_t eax = (base & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
    
    // If more than 32 bits can be used for an address then set edx to 
    // The maximum four bits of the 24 (+12) bit APIC base address
#ifdef __PHYSICAL_MEMORY_EXTENSION__
        edx = (base >> 32) & 0x0F;
#endif

    msr_set(IA32_APIC_BASE_MSR, eax, edx);
}

uint32_t read_register(uint32_t reg_offset) {
    uint32_t volatile *reg = (uint32_t volatile *)(apic_base + reg_offset);
    return *reg;
}

void write_register(uint32_t reg_offset, uint32_t value) {
    uint32_t volatile *reg = (uint32_t volatile *)(apic_base + reg_offset);
    *reg = value;
}

void apic_initialize() {
    // if (!is_apic_supported()) {
    //     printf("APIC is not Supported\n");
    //     return;
    // }


    // pic_configure(0x20, 0x28, false);
    // pic_disable();
    // printf("Disabled PIC\n");
    
    // apic_base = get_apic_base();

    // // Set the apic base to ensure enabled bit is set
    // set_apic_base(apic_base);

    // // Set the Spurious Interrupt Vector Register to 0x1FF
    // // 0x100 to enable the APIC and 0xFF as the spurious interrupt number.
    // write_register(0xF0, 0x1FF);

    // printf("Enabled APIC\n");
}
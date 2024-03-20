#include "hal.h"

#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/irq.h>
#include <arch/i686/isr.h>
#include <arch/i686/acpi.h>
#include <arch/i686/ps2.h>
#include <arch/i686/pci.h>

void hal_initialize(BootData *boot_data) {
    gdt_initialize();
    idt_initialize();
    isr_initialize();
    irq_initialize();
    acpi_initialize();
    ps2_initialize();
    pci_initialize(boot_data->pci_v2_installed, boot_data->pci_characteristics);
}
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t func;
    uint8_t class_code;
    uint8_t subclass_code;
} PCI_Device;

void pci_initialize(bool v2_installed, uint8_t flags);
uint32_t pci_dev_read_config_reg(PCI_Device *dev, uint8_t reg);
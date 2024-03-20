#include "pci.h"

#include "io.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

static enum {
    PORT_CONFIG_ADDR = 0xCF8,
    PORT_CONFIG_DATA = 0xCFC
} Ports;

static enum {
    VENDOR_ID = 0x0,
    DEVICE_ID = 0x2,
    COMMAND = 0x4,
    STATUS = 0x6,
    REVISION_ID = 0x8,
    PROG_IF = 0x9,
    SUBCLASS = 0xA,
    CLASS = 0xB,
    CACHE_LINE_SIZE = 0xC,
    LATENCY_TIME = 0xD,
    HEADER_TYPE = 0xE,
    BIST = 0xF,
    SECONDARY_BUS = 0x1A
} RegisterOffsets;

static bool config_method_1;

static void check_bus(uint8_t bus);

static uint32_t config_read_reg(
    uint8_t bus, uint8_t device, uint8_t func, uint8_t offset
) {
    uint32_t address;
    uint32_t lbus = bus;
    uint32_t ldevice = device;
    uint32_t lfunc = func;

    address = (lbus << 16) | (ldevice << 11) | (lfunc << 8) | (offset & 0xFC) |
        0x80000000;

    out_double(PORT_CONFIG_ADDR, address);
    
    int temp = in_double(PORT_CONFIG_DATA);
    return temp >> ((offset & 0x3) * 8);
}
static void check_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint8_t class = config_read_reg(bus, device, function, CLASS);
    uint8_t subclass = config_read_reg(bus, device, function, SUBCLASS);
    printf("  PCI Device [bus=%#x, dev=%#x, func=%#x, class=%#x, subclass=%#x]\n",
        bus, device, function, class, subclass
    );
    if (class == 0x6 && subclass == 0x4) {
        uint8_t bus2 = config_read_reg(bus, device, function, SECONDARY_BUS);
        check_bus(bus2);
    }
}

static void check_device(uint8_t bus, uint8_t device) {
    uint8_t function = 0;
    uint16_t vendor_id;
    uint8_t header_type;

    vendor_id = config_read_reg(bus, device, function, VENDOR_ID);
    if (vendor_id == 0xFFFF) return;
    check_function(bus, device, function);
    header_type = config_read_reg(bus, device, function, HEADER_TYPE);
    if ((header_type & 0x80) != 0) {
        for (function = 1; function < 8; function++) {
            vendor_id = config_read_reg(bus, device, function, VENDOR_ID);
            if (vendor_id != 0xFFFF) {
                check_function(bus, device, function);
            }
        }
    }
}

static void check_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        check_device(bus, device);
    }
}

void pci_initialize(bool v2_installed, uint8_t flags) {
    printf("Initializing PCI\n");
    config_method_1 = v2_installed && (flags & 0x1);
    if (!config_method_1) {
        printf("  PCI Configuration Method 1 Not supported.");
    }
    check_bus(0);
    printf("Initialized PCI\n");
}
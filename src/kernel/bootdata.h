#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t FirstAvailiableMemory;
    uint32_t MemoryMapAddr;
    uint32_t MemRegionCount;
    uint8_t MemRegionStructSize;
    uint8_t DiskCode; 
    bool pci_v2_installed;
    uint8_t pci_characteristics;
} __attribute__((packed)) BootData;

typedef struct {
    uint64_t BaseAddress;
    uint64_t Length;
    uint32_t Type;
    uint32_t ExtendedAttributes;
} __attribute__((packed)) MemoryRegion;
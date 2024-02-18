#pragma once

#include <stdint.h>

typedef struct {
    uint32_t Reserved;
    uint32_t MemoryMapAddr;
    uint32_t MemRegionCount;
    uint8_t MemRegionStructSize;
    uint8_t DiskCode; 
} __attribute__((packed)) BootData;

typedef struct {
    uint64_t BaseAddress;
    uint64_t Length;
    uint32_t Type;
    uint32_t ExtendedAttributes;
} __attribute__((packed)) MemoryRegion;
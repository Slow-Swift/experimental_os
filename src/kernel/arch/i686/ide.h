#pragma once

#include <arch/i686/pci.h>

typedef enum {
    ATA_TYPE_PATA,
    ATA_TYPE_PATAPI,
    ATA_TYPE_SATAPI,
    ATA_TYPE_SATA,
    ATA_TYPE_UNKOWN,
} ATA_DriveType;

typedef struct {
    bool present;           // Is device present
    int channel;            // 0 (Primary Channel) or 1 (Secondary Channel)
    int drive;              // 0 (Master) or 1 (Slave)
    ATA_DriveType type;     // Drive Type (ATA, ATAP, SATA)         
    uint64_t size;          // Number of sectors
    bool lba_48_supported;  // Is 48-bit addressing supported
    char model[41];         // Model of the disk
    int (*read_sectors)(uint8_t sectors, uint32_t lba, void * buffer);
} ATA_Drive;

void ide_initialize(PCI_Device *dev);
ATA_Drive * ide_get_drive(int drive);
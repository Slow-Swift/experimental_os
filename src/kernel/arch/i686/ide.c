#include "ide.h"

#include <arch/i686/io.h>
#include <arch/i686/pci.h>
#include <stdio.h>

typedef enum {
    ATA_SR_BSY    = 0x80,    // Busy
    ATA_SR_RDY   = 0x40,    // Drive ready
    ATA_SR_DF     = 0x20,    // Drive write fault
    ATA_SR_DSC    = 0x10,    // Drive seek complete
    ATA_SR_DRQ    = 0x08,    // Data request ready
    ATA_SR_CORR   = 0x04,    // Corrected data
    ATA_SR_IDX    = 0x02,    // Index
    ATA_SR_ERR    = 0x01,    // Error
} ATA_Status;

typedef enum {
    ATA_ER_BBK    = 0x80,    // Bad block
    ATA_ER_UNC    = 0x40,    // Uncorrectable data
    ATA_ER_MC     = 0x20,    // Media changed
    ATA_ER_IDNF   = 0x10,    // ID mark not found
    ATA_ER_MCR    = 0x08,    // Media change request
    ATA_ER_ABRT   = 0x04,    // Command aborted
    ATA_ER_TK0NF  = 0x02,    // Track 0 not found
    ATA_ER_AMNF   = 0x01,    // No address mark
} ATA_Error;

typedef enum {
    ATA_CMD_READ_PIO         = 0x20,
    ATA_CMD_READ_PIO_EXT     = 0x24,
    ATA_CMD_READ_DMA         = 0xC8,
    ATA_CMD_READ_DMA_EXT     = 0x25,
    ATA_CMD_WRITE_PIO        = 0x30,
    ATA_CMD_WRITE_PIO_EXT    = 0x34,
    ATA_CMD_WRITE_DMA        = 0xCA,
    ATA_CMD_WRITE_DMA_EXT    = 0x35,
    ATA_CMD_CACHE_FLUSH      = 0xE7,
    ATA_CMD_CACHE_FLUSH_EXT  = 0xEA,
    ATA_CMD_PACKET           = 0xA0,
    ATA_CMD_IDENTIFY_PACKET  = 0xA1,
    ATA_CMD_IDENTIFY         = 0xEC,
    ATAPI_CMD_READ           = 0xA8,
    ATAPI_CMD_EJECT          = 0x1B
} ATA_Command;

typedef enum {
    ATA_IDEN_MODEL = 27 * 2,
    ATA_IDEN_FEATURES = 83 * 2,
    ATA_IDEN_SIZE_28 = 60 * 2,
    ATA_IDEN_SIZE_48 = 100 * 2
} ATA_IdentifierSpace;

typedef enum {
    IDE_ATA        = 0x00,
    IDE_ATAPI      = 0x01,
    ATA_MASTER     = 0x00,
    ATA_SLAVE      = 0x01
} ATA_DevType;

typedef enum {
    ATA_REG_DATA       = 0x00,
    ATA_REG_ERROR      = 0x01,
    ATA_REG_FEATURES   = 0x01,
    ATA_REG_SECCOUNT0  = 0x02,
    ATA_REG_LBA0       = 0x03,
    ATA_REG_LBA1       = 0x04,
    ATA_REG_LBA2       = 0x05,
    ATA_REG_HDDEVSEL   = 0x06,
    ATA_REG_COMMAND    = 0x07,
    ATA_REG_STATUS     = 0x07,
    ATA_REG_SECCOUNT1  = 0x08,
    ATA_REG_LBA3       = 0x09,
    ATA_REG_LBA4       = 0x0A,
    ATA_REG_LBA5       = 0x0B,
    ATA_REG_CONTROL    = 0x00,
    ATA_REG_ALTSTATUS  = 0x00,
    ATA_REG_DEVADDRESS = 0x01,
} ATA_Registers;

typedef enum {
    ATA_PRIMARY     = 0x00,
    ATA_SECONDARY   = 0x01
} ATA_Channel;

struct ChannelRegisters {
    uint8_t selected_dev;
    uint16_t base;
    uint16_t ctrl;
    uint16_t bus_master_ide;
    bool no_interrupt;
} channels[2];

ATA_Drive drives[4];

static uint8_t ide_buffer[512] = {0};
volatile static uint8_t ide_irq_invoked = 0;
static uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void select_device(ATA_Drive *drive) {
    if (channels[drive->channel].selected_dev != drive->drive) {
        channels[drive->channel].selected_dev = drive->drive;
        uint16_t port = channels[drive->channel].base + ATA_REG_HDDEVSEL;
        uint8_t devsel = (drive->drive == 0) ? 0xA0 : 0xB0;
        out_byte(port, devsel);

        // 400 ns delay
        uint8_t status;
        for (int i=0; i<14; i++) 
            status = in_byte(channels[drive->channel].ctrl + ATA_REG_ALTSTATUS);
    }
}

static void wait_dev_nbsy(ATA_Drive *drive) {
    uint8_t status;
    do {
        status = in_byte(channels[drive->channel].ctrl + ATA_REG_ALTSTATUS);
    } while(((status & ATA_SR_BSY) != 0));
}

static int read_sectors(
    uint8_t sectors, uint32_t lba, void *buffer, ATA_Drive *drive
) {
    if (!drive->present) {
        printf("ERROR: Invalid drive\n");
        return 0;
    }

    if (lba > 0x0FFFFFFF) {
        printf("ERROR: LBA %d too high for 28 bit addressing\n");
        return 0;
    }

    uint16_t base = channels[drive->channel].base;
    uint16_t ctrl = channels[drive->channel].ctrl;
    uint8_t drive_select = 0xE0 | (drive->drive << 4) | ((lba >> 24) & 0x0F);
    out_byte(base + ATA_REG_HDDEVSEL, drive_select);
    out_byte(base + ATA_REG_SECCOUNT0, sectors);
    out_byte(base + ATA_REG_LBA0, lba);
    out_byte(base + ATA_REG_LBA1, lba >> 8);
    out_byte(base + ATA_REG_LBA2, lba >> 16);
    wait_dev_nbsy(drive);
    out_byte(base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    // Read in the sectors
    uint16_t *u16_buff = (uint16_t *)buffer;
    for (int s=0; s<sectors; s++) {
        // Wait for status to indicate ready or error
        uint8_t status = in_byte(ctrl + ATA_REG_ALTSTATUS);
        while (
            ((status & ATA_SR_BSY) || !(status & ATA_SR_DRQ)) &&
            !(status & ATA_SR_ERR) && !(status & ATA_SR_DF)
        )
            status = in_byte(ctrl + ATA_REG_ALTSTATUS);

        // If error then exit
        if ((status & ATA_SR_ERR) || (status & ATA_SR_DF)) {
            return s;
        }

        // Read in the sector
        for (int i=0; i<256; i++) {
            u16_buff[i] = in_word(base + ATA_REG_DATA);
        }
        u16_buff += 256;

        // Waste 400ns for status to reset
        for (int i=0; i<14; i++) 
            in_byte(channels[drive->channel].ctrl + ATA_REG_ALTSTATUS);
    }

}

static int read_sectors_d0(uint8_t sectors, uint32_t lba, void *buffer) {
    return read_sectors(sectors, lba, buffer, &drives[0]);
}

static int read_sectors_d1(uint8_t sectors, uint32_t lba, void *buffer) {
    return read_sectors(sectors, lba, buffer, &drives[1]);
}

static int read_sectors_d2(uint8_t sectors, uint32_t lba, void *buffer) {
    return read_sectors(sectors, lba, buffer, &drives[2]);
}

static int read_sectors_d3(uint8_t sectors, uint32_t lba, void *buffer) {
    return read_sectors(sectors, lba, buffer, &drives[3]);
}

static void initialize_drive(ATA_Drive *drive) {
    uint16_t base = channels[drive->channel].base;
    uint16_t ctrl = channels[drive->channel].ctrl;
    drive->present = false;

    // Select the device
    select_device(drive);

    // Send the identify command
    out_byte(base + ATA_REG_SECCOUNT0, 0);
    out_byte(base + ATA_REG_LBA0, 0);
    out_byte(base + ATA_REG_LBA1, 0);
    out_byte(base + ATA_REG_LBA2, 0);
    out_byte(base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    // Check if device is present
    uint8_t status = in_byte(ctrl + ATA_REG_ALTSTATUS);
    if (status == 0) return;

    drive->present = true;

    // Poll until not busy
    while (status & ATA_SR_BSY) status = in_byte(ctrl + ATA_REG_ALTSTATUS);


    // Determine drive type
    int sig_low = in_byte(base + ATA_REG_LBA1);
    int sig_high = in_byte(base + ATA_REG_LBA2);
    if (sig_low == 0 && sig_high == 0) drive->type = ATA_TYPE_PATA;
    else if (sig_low == 0x14 && sig_high == 0xEB) drive->type = ATA_TYPE_PATAPI;
    else if (sig_low == 0x69 && sig_high == 0x96) drive->type = ATA_TYPE_SATAPI;
    else if (sig_low == 0x3c && sig_high == 0xc3) drive->type = ATA_TYPE_SATA;
    else drive->type = ATA_TYPE_UNKOWN;
    
    // If Not PATA then don't do anything
    if (drive->type != ATA_TYPE_PATA) {
        drive->lba_48_supported = false;
        drive->size = 0;
        drive->present = false;
        return;
    }

    // Poll until data ready
    while (!(status & ATA_SR_DRQ) && !(status & ATA_SR_ERR)) 
        status = in_byte(ctrl + ATA_REG_ALTSTATUS);

    // Error set
    if (status & ATA_SR_ERR) return;

    // Read in the packet
    for (int i = 0; (status & ATA_SR_DRQ) && (i < 512); i+=2) {
        uint16_t word = in_word(base + ATA_REG_DATA);
        ide_buffer[i] = word & 0xFF;
        ide_buffer[i+1] = word >> 8;
        status = in_byte(ctrl + ATA_REG_ALTSTATUS);
    }

    uint16_t features = *(uint16_t*)(ide_buffer + ATA_IDEN_FEATURES);
    drive->lba_48_supported = features & (1 << 10);

    if (drive->lba_48_supported) {
        drive->size = *((uint64_t *)(ide_buffer + ATA_IDEN_SIZE_48));
        if (drive->size == 0) drive->lba_48_supported = false;
    }

    if (!(drive->lba_48_supported)) {
        drive->size = *((uint32_t *)(ide_buffer + ATA_IDEN_SIZE_28));
    }

    for (int i=0; i<40; i+=2) {
        drive->model[i] = ide_buffer[ATA_IDEN_MODEL + i + 1];
        drive->model[i + 1] = ide_buffer[ATA_IDEN_MODEL + i];
    }
    drive->model[40] = '\0';
}

static void initialize_channels(PCI_Device *dev) {
    uint8_t prog_if = pci_dev_read_config_reg(dev, 0x9);

    // No devices selected
    channels[0].selected_dev = 0xFF;
    channels[1].selected_dev = 0xFF;

    // Use default port addresses
    channels[0].base = 0x1F0;
    channels[0].ctrl = 0x3F6;
    channels[1].base = 0x170;
    channels[1].ctrl = 0x376;

    // Read in port addresses if specified
    if ((prog_if & 0x1) != 0) {
        channels[0].base = pci_dev_read_config_reg(dev, 0x10);
        channels[0].ctrl = pci_dev_read_config_reg(dev, 0x14);
    }

    // Read in port addresses if specified
    if ((prog_if & 0x4) != 0) {
        channels[1].base = pci_dev_read_config_reg(dev, 0x18);
        channels[1].ctrl = pci_dev_read_config_reg(dev, 0x1C);
    }
}

static void initialize_drives() {
    drives[0].read_sectors = read_sectors_d0;
    drives[1].read_sectors = read_sectors_d1;
    drives[2].read_sectors = read_sectors_d2;
    drives[3].read_sectors = read_sectors_d3;

    for (int c=0; c<2; c++) {
        for (int d=0; d<2; d++) {
            ATA_Drive *drive = drives + 2*c + d;
            drive->channel = c;
            drive->drive = d;
            initialize_drive(drive);
        }
    }

    for (int i=0; i<4; i++) {
        if (drives[i].present) {
            printf("  Found %s Drive %lldMB - %s\n", 
                (const char *[]){"PATA", "PATAPI", "SATAPI", "SATA", "UNKOWN"}[drives[i].type],
                drives[i].size / 2 / 1024,
                drives[i].model
            );
        }
    }
}

ATA_Drive * ide_get_drive(int drive) {
    if (drives[drive].present) return drives + drive;
    return NULL;
}

void ide_initialize(PCI_Device *dev) {
    if (dev->class_code != 0x01 || dev->subclass_code != 0x01) return;

    printf("  Initializing IDE Device\n");

    initialize_channels(dev);
    initialize_drives();

    printf("  Initialized IDE Device\n");
}
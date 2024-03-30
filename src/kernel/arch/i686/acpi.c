#include "acpi.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>

static char rds_str[] = "RSD PTR ";

typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) E_RSDP;

typedef struct {
    ACPI_SDT_Header header;
    ACPI_SDT_Header *entries;
} RSDT;

static RSDT* rsdt = NULL;
static FADT* fadt = NULL;
static bool use_xsdt = false;

static const int RSDP_SIZE = 0x8 + 0x1 + 0x6 + 0x1 + 0x4;

static bool is_rsdp_valid(E_RSDP *rsdp) {
    int version = rsdp->revision;
    if (version != 0 && version != 2) return false;

    int checksum = 0;
    for (int i=0; i<RSDP_SIZE; i++) 
        checksum += rsdp->signature[i];
    if ((checksum & 0xFF) != 0) return false;

    if (version == 2) {
        checksum = 0;
        for (int i=0; i<sizeof(E_RSDP); i++) 
            checksum += rsdp->signature[i];
        if ((checksum & 0xFF) != 0) return false;
    }

    return true;
}

static void *find_table(const char* signature) {
    int entries;

    // Divide length by the size of an pointer to determine entry count
    if (use_xsdt)
        entries = (rsdt->header.length - sizeof(rsdt->header)) / 8;
    else
        entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

    ACPI_SDT_Header *header;
    for (int i = 0; i < entries; i++) {
        //* If this is 64-bit code this may have to be changed since
        //* pointers will already be 64 bits and there will be no need to
        //* multiply by 2
        if (use_xsdt)
            header = &(rsdt->entries[i*2]);
        else
            header = &(rsdt->entries[i]);
        if (memcmp(header->signature, signature, 4) == 0) return header;
    }

    return NULL;
}

static bool has_valid_checksum(ACPI_SDT_Header *table_header) {
    unsigned char sum;
    for (int i = 0; i < table_header->length; i++) {
        sum += ((char *)table_header)[i];
    }

    return sum == 0;
}

static E_RSDP *find_rsdp(int mem_min, int mem_max) 
{
    bool found = false;
    uintptr_t current = (mem_min + 0xF) / 0x10;
    while (current < mem_max) {
        if (memcmp((void*)current, rds_str, 8) == 0) {
            if (is_rsdp_valid((E_RSDP *)current)) {
                found = true;
                break;
            }
        }
        current += 16;
    }

    if (found) return (E_RSDP *)current;
    return NULL;
}

FADT* get_fadt() {
    return fadt;
}

void acpi_initialize() {
    const uint16_t *ebda_ptr = (uint16_t *)0xA0E;
    const uint16_t *ebda_length = (uint16_t *)0xA13;

    E_RSDP *rsdp = find_rsdp(*ebda_ptr * 16, *ebda_ptr * 16 + *ebda_length);
    if (rsdp == NULL)
     rsdp = find_rsdp(0xE0000, 0x00100000);

    if (rsdp == NULL) {
        panic("ACPI", "Could not find RSDP");
        // no return
    }
    
    printf("Found RSDP at %p\n", rsdp);
    printf("RSDP Version: %d\n", rsdp->revision);
    printf("RSDT Address: %#x\n", rsdp->rsdt_address);
    printf("XSDT Address: %#x\n", rsdp->xsdt_address);

    if (rsdp->revision == 0) {
        rsdt = (RSDT *)rsdp->rsdt_address;
        use_xsdt = false;
        printf("  Using RSDT\n");
    } else {
        rsdt = (RSDT *)(uint32_t)rsdp->xsdt_address;
        use_xsdt = true;
        printf("  Using XSDT\n");
    }

    
    if (!has_valid_checksum(&rsdt->header)) {
        panic("ACPI", "Invalid RSDT Table");
    }

    fadt = find_table("FACP");
    if (!has_valid_checksum(&fadt->header)) {
        panic("ACPI", "Invalid FADT");
    }

    printf("FADT Bytes: \n");
    hexdump(stdout, fadt, sizeof(FADT));
    printf("FACS Bytes: \n");
    hexdump(stdout, (void *)(fadt->firmware_ctrl), 32);
    printf("ACPI Version: %d\n", fadt->header.revision);
}
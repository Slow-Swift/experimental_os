#include "i8259.h"
#include "io.h"
#include <stdbool.h>

#define PIC1_COMMAND_PORT       0x20
#define PIC1_DATA_PORT          0x21
#define PIC2_COMMAND_PORT       0xA0
#define PIC2_DATA_PORT          0xA1

enum {
    PIC_ICW1_IC4            = 0x01,
    PIC_ICW1_SINGLE         = 0x02,
    PIC_ICW1_INTERVAL4      = 0x04,
    PIC_ICW1_LEVEL          = 0x08,
    PIC_ICW1_Initialize     = 0x10
} PIC_ICW1;

enum {
    PIC_ICW4_8086           = 0x01,
    PIC_ICW4_AUTO_EOI       = 0x02,
    PIC_ICW4_BUFFER_MASTER  = 0x04,
    PIC_ICW4_BUFFER_SLAVE   = 0x00,
    PIC_ICW4_BUFFERED       = 0x08,
    PIC_ICW4_SFNM           = 0x10
} PIC_ICW4;

enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
} PIC_CMD;

static uint16_t g_PicMask = 0xFFFF;
static bool g_AutoEoi = false;

void i8259_SetMask(uint16_t newMask) {
    g_PicMask = newMask;
    i686_outb(PIC1_DATA_PORT, newMask & 0xFF);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, newMask >> 8);
    i686_iowait();
}

uint16_t i8259_GetMask() {
    return i686_inb(PIC1_DATA_PORT) | (i686_inb(PIC2_DATA_PORT) << 8);
}

void i8259_Configure(uint8_t offsetPic1, uint8_t offsetPic2, bool autoEoi) {

    // Mask everything
    i8259_SetMask(0xFFFF);

    // Initialization control word 1
    i686_outb(PIC1_COMMAND_PORT, PIC_ICW1_IC4 | PIC_ICW1_Initialize);
    i686_iowait();
    i686_outb(PIC2_COMMAND_PORT, PIC_ICW1_IC4 | PIC_ICW1_Initialize);
    i686_iowait();

    // Initailization control word 2
    i686_outb(PIC1_DATA_PORT, offsetPic1);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, offsetPic2);
    i686_iowait();

    // Initialization control word 3
    i686_outb(PIC1_DATA_PORT, 0x4);         // Tell PIC1 that it has a slave at IRQ2 (0000 0100)
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, 0x2);         // Tell PIC2 it's cascade identity (0000 0010)
    i686_iowait();

    // Initialization control word 4
    uint8_t icw4 = PIC_ICW4_8086;
    if (autoEoi)
        icw4 |= PIC_ICW4_AUTO_EOI;
    g_AutoEoi = autoEoi;

    i686_outb(PIC1_DATA_PORT, icw4);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, icw4);
    i686_iowait();

    // Clear data registers
    i686_outb(PIC1_DATA_PORT, 0);
    i686_iowait();
    i686_outb(PIC2_DATA_PORT, 0);
    i686_iowait();

    // Mask all interrupts until they are enabled by device drivers
    i8259_SetMask(0xFFFF);
}

void i8259_SendEndOfInterrupt(int irq) {
    if (irq >= 8)
        i686_outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
    i686_outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
}

void i8259_Disable() {
    i8259_SetMask(0xFFFF);
}

void i8259_Mask(int irq) {
    i8259_SetMask(g_PicMask | (1 << irq));
}

void i8259_Unmask(int irq) {
    i8259_SetMask(g_PicMask & ~(1 << irq));
}

uint16_t i8259_ReadIRQRequestRegister() {
    i686_outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    i686_outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    return (i686_inb(PIC1_COMMAND_PORT) | (i686_inb(PIC2_COMMAND_PORT) << 8));
}

uint16_t i8259_ReadInServiceRegister() {
    i686_outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    i686_outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    return (i686_inb(PIC1_COMMAND_PORT) | (i686_inb(PIC2_COMMAND_PORT) << 8));
}

bool i8259_Probe() {
    i8259_Disable();

    // Magic numbers that don't mean anything. Just must match.
    i8259_SetMask(0x1337);
    return i8259_GetMask() == 0x1337;
}

static const PICDriver g_PicDriver = {
    .Name = "8259 PIC",
    .Probe = &i8259_Probe,
    .Initialize = &i8259_Configure,
    .Disable = &i8259_Disable,
    .SendEndOfInterrupt = &i8259_SendEndOfInterrupt,
    .Mask = &i8259_Mask,
    .Unmask = &i8259_Unmask
};

const PICDriver* i8259_GetDriver() {
    return &g_PicDriver;
}

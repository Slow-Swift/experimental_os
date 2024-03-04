#include "i8259.h"
#include "io.h"
#include <stdbool.h>

static enum {
    PIC_PORT_CMD_1  = 0x20,
    PIC_PORT_DATA_1 = 0x21,
    PIC_PORT_CMD_2  = 0xA0,
    PIC_PORT_DATA_2 = 0xA1
} PIC_PORT;

static enum {
    PIC_ICW1_IC4            = 0x01,
    PIC_ICW1_SINGLE         = 0x02,
    PIC_ICW1_INTERVAL4      = 0x04,
    PIC_ICW1_LEVEL          = 0x08,
    PIC_ICW1_Initialize     = 0x10
} PIC_ICW1;

static enum {
    PIC_ICW4_8086           = 0x01,
    PIC_ICW4_AUTO_EOI       = 0x02,
    PIC_ICW4_BUFFER_MASTER  = 0x04,
    PIC_ICW4_BUFFER_SLAVE   = 0x00,
    PIC_ICW4_BUFFERED       = 0x08,
    PIC_ICW4_SFNM           = 0x10
} PIC_ICW4;

static enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
} PIC_CMD;

static uint16_t pic_mask = 0xFFFF;

static void set_mask(uint16_t new_mask) {
    pic_mask = new_mask;
    out_byte(PIC_PORT_DATA_1, new_mask & 0xFF);
    io_wait();
    out_byte(PIC_PORT_DATA_2, new_mask >> 8);
    io_wait();
}

static uint16_t get_mask() {
    return in_byte(PIC_PORT_DATA_1) | (in_byte(PIC_PORT_DATA_2) << 8);
}

static void configure(uint8_t offset_pic1, uint8_t offset_pic2, bool auto_eoi) {

    // mask everything
    set_mask(0xFFFF);

    // Initialization control word 1
    out_byte(PIC_PORT_CMD_1, PIC_ICW1_IC4 | PIC_ICW1_Initialize);
    io_wait();
    out_byte(PIC_PORT_CMD_2, PIC_ICW1_IC4 | PIC_ICW1_Initialize);
    io_wait();

    // Initailization control word 2
    out_byte(PIC_PORT_DATA_1, offset_pic1);
    io_wait();
    out_byte(PIC_PORT_DATA_2, offset_pic2);
    io_wait();

    // Initialization control word 3
    out_byte(PIC_PORT_DATA_1, 0x4);         // Tell PIC1 that it has a slave at IRQ2 (0000 0100)
    io_wait();
    out_byte(PIC_PORT_DATA_2, 0x2);         // Tell PIC2 it's cascade identity (0000 0010)
    io_wait();

    // Initialization control word 4
    uint8_t icw4 = PIC_ICW4_8086;
    if (auto_eoi)
        icw4 |= PIC_ICW4_AUTO_EOI;

    out_byte(PIC_PORT_DATA_1, icw4);
    io_wait();
    out_byte(PIC_PORT_DATA_2, icw4);
    io_wait();

    // Clear data registers
    out_byte(PIC_PORT_DATA_1, 0);
    io_wait();
    out_byte(PIC_PORT_DATA_2, 0);
    io_wait();

    // mask all interrupts until they are enabled by device drivers
    set_mask(0xFFFF);
}

static void send_eoi(int irq) {
    if (irq >= 8)
        out_byte(PIC_PORT_CMD_2, PIC_CMD_END_OF_INTERRUPT);
    out_byte(PIC_PORT_CMD_1, PIC_CMD_END_OF_INTERRUPT);
}

static void disable() {
    set_mask(0xFFFF);
}

static void mask(int irq) {
    set_mask(pic_mask | (1 << irq));
}

static void unmask(int irq) {
    set_mask(pic_mask & ~(1 << irq));
}

static uint16_t read_irr() {
    out_byte(PIC_PORT_CMD_1, PIC_CMD_READ_IRR);
    out_byte(PIC_PORT_CMD_2, PIC_CMD_READ_IRR);
    return (in_byte(PIC_PORT_CMD_1) | (in_byte(PIC_PORT_CMD_2) << 8));
}

static uint16_t read_isr() {
    out_byte(PIC_PORT_CMD_1, PIC_CMD_READ_ISR);
    out_byte(PIC_PORT_CMD_2, PIC_CMD_READ_ISR);
    return (in_byte(PIC_PORT_CMD_1) | (in_byte(PIC_PORT_CMD_2) << 8));
}

static bool probe() {
    disable();

    // Magic numbers that don't mean anything. Just must match.
    set_mask(0x1337);
    return get_mask() == 0x1337;
}

static const PIC_Driver driver = {
    .name = "8259 PIC",
    .probe = &probe,
    .initialize = &configure,
    .disable = &disable,
    .send_eoi = &send_eoi,
    .mask = &mask,
    .unmask = &unmask
};

const PIC_Driver* i8259_get_driver() {
    return &driver;
}

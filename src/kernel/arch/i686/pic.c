#include "pic.h"

#include "io.h"

#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0x20
#define PIC2_DATA       0xA1

#define INITIALIZE_PIC  0x11
#define PIC_8086_MODE   0x01
#define PIC_AUTO_EOI    0x02
#define PIC_EOI         0x20
#define PIC_READ_IRR    0x0A
#define PIC_READ_ISR    0x0B

static void pic8259_set_mask(uint16_t mask) {
    out_byte(PIC1_DATA, mask & 0xFF);
    out_byte(PIC2_DATA, mask >> 8);
}

void pic_configure(int offset1, int offset2, bool auto_eoi)
{
    uint8_t a1, a2;

    // Save interrupt masks
    a1 = in_byte(PIC1_DATA);
    a2 = in_byte(PIC2_DATA);

    // Begin initialization process
    out_byte(PIC1_COMMAND, INITIALIZE_PIC);
    out_byte(PIC2_COMMAND, INITIALIZE_PIC);

    // Load the offsets
    out_byte(PIC1_DATA, offset1);
    out_byte(PIC2_DATA, offset2);

    // Load Master/Slave information
    out_byte(PIC1_DATA, 4); // Tell Master PIC that there is a slave 
                            // PIC at IRQ2 (0000 0100)
    out_byte(PIC2_DATA, 2); // Tell Slave PIC its cascade identity (0000 0010)

    // Enable 8086 mode and auto eoi (if requested) on both PICs
    uint8_t mode = PIC_8086_MODE;
    if (auto_eoi) mode |= PIC_AUTO_EOI;

    out_byte(PIC1_DATA, mode);
    out_byte(PIC2_DATA, mode);

    // Restore saved masks
    out_byte(PIC1_DATA, a1);
    out_byte(PIC2_DATA, a2);
}

void pic_mask(int irq_line) {
    uint16_t port;
    uint8_t value;

    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    value = in_byte(port) | (1 << irq_line);
    out_byte(port, value);
}

void pic_unmask(int irq_line) {
    uint16_t port;
    uint8_t value;

    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    value = in_byte(port) | ~(1 << irq_line);
    out_byte(port, value);
}

uint16_t pic_get_irr() {
    out_byte(PIC1_COMMAND, PIC_READ_IRR);
    out_byte(PIC2_COMMAND, PIC_READ_IRR);
    return (in_byte(PIC2_COMMAND) << 8) | in_byte(PIC1_COMMAND);
}

uint16_t pic_get_isr() {
    out_byte(PIC1_COMMAND, PIC_READ_IRR);
    out_byte(PIC2_COMMAND, PIC_READ_IRR);
    return (in_byte(PIC2_COMMAND) << 8) | in_byte(PIC1_COMMAND);
}

void pic_send_EOI(int irq_line) {
    if (irq_line >= 8)
        out_byte(PIC2_COMMAND, PIC_EOI);

    out_byte(PIC1_COMMAND, PIC_EOI);
}

void pic_disable() {
    pic_mask(0xFFFF);
}
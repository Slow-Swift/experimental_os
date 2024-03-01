#include "isr.h"

#include <debug.h>
#include "idt.h"
#include <stddef.h>
#include <stdio.h>

ISR_Handler handlers[256];

static const char* const exception_names[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Availiable",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void isr_handle_interrupt(Registers* regs) {
    log_debug("ISR", "Interrup Called");
    if (handlers[regs->interrupt] != NULL) {
        handlers[regs->interrupt](regs);
    } else if (regs->interrupt >= 0x20) {
        printf("Unhandled interrupt %x!\n", regs->interrupt);
    } else {
        panic(
            "ISR", 
            "Unhandled exception %x, %s!\n" \
            "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n" \
            "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n" \
            "  interrupt=%x  errorcode=%x\n",
            regs->interrupt, exception_names[regs->interrupt],
            regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi,
            regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, 
            regs->ss, regs->interrupt, regs->error
        );     
    }
}

void isr_register_handler(int interrupt, ISR_Handler handler) {
    handlers[interrupt] = handler;
    idt_enable_gate(interrupt);
}

void isr_initialize_gates();

void isr_initialize() {
    isr_initialize_gates();
    for (int i = 0; i < 256; i++) {
        idt_enable_gate(i);
    }
}
#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "io.h"
#include <stddef.h>
#include <stdio.h>

static ISR_Handler isr_handlers[256];

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

void isr_initialize_gates();

void isr_initialize() {
    isr_initialize_gates();
    for (int i = 0; i < 256; i++)
        idt_enable_gate(i);
}

void __attribute__((cdecl)) isr_handler_common(Registers *regs) {
    if (isr_handlers[regs->interrupt] != NULL)
        isr_handlers[regs->interrupt](regs);
    else if (regs->interrupt >= 32)
        printf("Unhandled interrupt %d!\n", regs->interrupt);
    else {
        printf("Unhandled exception %d %s\n", regs->interrupt, exception_names[regs->interrupt]);

        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
                regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
                regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        printf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);

        printf("KERNEL PANIC!\n");
        panic_stop();
    }
}

void isr_register_handler(int interrupt, ISR_Handler handler)
{
    isr_handlers[interrupt] = handler;
    idt_enable_gate(interrupt);
}

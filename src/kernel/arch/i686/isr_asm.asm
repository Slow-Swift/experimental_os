[bits 32]

extern i686_ISR_Handler

%macro ISR_NOERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
    push 0              ; push dummy error code
    push %1              ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
    ; error code pushed by interrupt
    push %1              ; push interrupt number
    jmp isr_common

%endmacro

%include "arch/i686/isrs_gen.inc"

isr_common:
    pusha           ; Save all registers
    
    xor eax, eax    ; Push ds
    mov ax, ds
    push eax

    mov ax, 0x10    ; Use kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp        ; Pass pointer to stack to C
    call i686_ISR_Handler
    add esp, 4

    pop eax         ; Restore old segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Restore all registers
    add esp, 8      ; Remove error code and interrupt number
    iret

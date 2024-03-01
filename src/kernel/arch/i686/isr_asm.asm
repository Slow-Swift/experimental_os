[bits 32]

extern isr_handle_interrupt

%macro ISR_NOERRORCODE 1

; Start handling an interrupt with no error code
global i686_isr%1:
i686_isr%1:
    push 0              ; push dummy error code
    push %1              ; push interrupt number
    jmp isr_common_handler

%endmacro

; Start handling an interrupt with an error code
%macro ISR_ERRORCODE 1

global i686_isr%1:
i686_isr%1:
    ; error code pushed by interrupt
    push %1              ; push interrupt number
    jmp isr_common_handler

%endmacro

%include "arch/i686/isrs_gen.inc"

isr_common_handler:
    ; Already pushed values
    ;   eflags  (by cpu)
    ;   cs      (by cpu)
    ;   eip     (by cpu)
    ;   error   (by cpu or dummy push)
    ;   interrupt (by prehandler)
    
    pusha   ; push eax, ecx, edx, ebx, esp, ebp, esi, edi

    ; push ds
    xor eax, eax
    mov eax, ds
    push eax

    ; Update segments to use kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Call the actual handler passing the stack pointer
    push esp     
    call isr_handle_interrupt
    add esp, 4

    ; Restore old segment
    pop eax         
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Restore all registers
    add esp, 8      ; Remove error code and interrupt number
    iret
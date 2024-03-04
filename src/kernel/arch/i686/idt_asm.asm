[bits 32]

; void __attribute__((cdecl)) idt_load(IDT_Descriptor* idt_descriptor);
global idt_load
idt_load:
    ; Make new callframe
    push ebp        ; Save old callframe
    mov ebp, esp    ; Initialize new callframe

    ; Load idt
    mov eax, [ebp + 8]  ; Interrupt descriptor table in arg[0]
    lidt [eax]

    ; Restore old callframe
    mov esp, ebp
    pop ebp
    ret

[bits 32]

; void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);
global i686_IDT_Load
i686_IDT_Load:
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

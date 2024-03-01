global idt_load
idt_load:
    ; Make new callframe
    push ebp
    mov ebp, esp

    ; load idt
    mov eax, [ebp + 8]  ; ldt address in arg[0]
    lidt [eax]

    ; Restore old callframe
    mov esp, ebp
    pop ebp
    ret
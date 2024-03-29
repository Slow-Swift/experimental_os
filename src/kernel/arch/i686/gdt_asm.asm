[bits 32]

;
; void gdt_load(GDT_Descriptor* descriptor, uint16_t code_segment, uint16_t data_segment);
;
global gdt_load
gdt_load:
    ; Make new callframe
    push ebp        ; Save old callframe
    mov ebp, esp    ; Initialize new callframe

    ; Load GDT
    mov eax, [ebp + 8]  ; GDT_Descriptor in arg[0]
    lgdt [eax]

    ; Reload code segment
    mov eax, [ebp + 12] ; Code segment in arg[1]
    push eax
    push .reload_cs
    retf

.reload_cs:

    ; Reload data segment
    mov ax, [ebp + 16] ; Data segment in arg[2]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Restore old callframe
    mov esp, ebp
    pop ebp
    ret

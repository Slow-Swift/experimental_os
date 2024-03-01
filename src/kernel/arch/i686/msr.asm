[bits 32]

global is_msr_present
is_msr_present:
    push ecx
    push ebx
    push edx

    xor eax, eax
    inc eax
    cpuid

    mov eax, 0b100000
    and eax, edx

    pop edx
    pop ebx
    pop ecx
    ret

global msr_get
msr_get:
    push ebp        ; Save old callframe
    mov ebp, esp    ; Initialize new callframe

    ; EAX, ECX, EDX caller saved

    mov ecx, [ebp + 8]  ; Load msr from arg[0]
    rdmsr

    mov ecx, [ebp + 12] ; Load low address from arg[1]
    mov [ecx], eax

    mov ecx, [ebp + 16] ; Load high address from arg[2]
    mov [ecx], edx

    ; Restore old callframe
    mov esp, ebp
    pop ebp
    ret

global msr_set
msr_set:
    push ebp        ; Save old callframe
    mov ebp, esp    ; Initialize new callframe

    mov ecx, [ebp + 8]  ; Load msr from arg[0]
    mov eax, [ebp + 12] ; Load low bytes from arg[1]
    mov edx, [ebp + 16] ; Load high bytes from arg[2]

    wrmsr

    ; Restore old callframe
    mov esp, ebp
    pop ebp
    ret
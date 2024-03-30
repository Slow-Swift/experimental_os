[bits 32]

%define UNUSED_PORT 0x08

global out_byte
out_byte:  
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global in_byte
in_byte: 
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global out_word
out_word:  
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

global in_word
in_word: 
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

global out_double
out_double:  
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

global in_double
in_double: 
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

global io_wait
io_wait:
    xor dx, dx
    mov al, UNUSED_PORT
    out dx, al
    ret

global disable_interrupts
disable_interrupts:
    cli
    ret

global enable_interrupts
enable_interrupts:
    sti
    ret

global panic_stop
panic_stop:
    cli
    hlt

global halt
halt:
    hlt
    ret
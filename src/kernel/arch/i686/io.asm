[bits 32]

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
bits 16

global load_gdt
global enter_unreal_mode
global enter_protected_mode

section .text

    ; 
    ; Load the global descriptor table
    ;
    load_gdt:
        cli
        lgdt [gdt_info]
        ret

    ;
    ; Enter protected mode
    ; eax: address to jump to
    ;
    enter_protected_mode:
        mov ebx, eax
        mov eax, cr0
        or al, 1
        mov cr0, eax
        jmp 0x8:.pmode
        
    .pmode:
        [bits 32]
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax

        cli
        call ebx

        ret
        [bits 16]

    ; 
    ; Switch into unreal mode 
    ;
    enter_unreal_mode:
        push eax
        push bx
        push ds

        ; Switch to protected mode
        mov eax, cr0    
        or al, 1        ; Set pmode bit
        mov cr0, eax
        jmp 0x18:.pmode

    .pmode:
        mov bx, 0x10    ; Select unreal data descriptor
        mov ds, bx

        ; Swich out of protected mode
        and al, 0xFE    ; Disable pmode bit
        mov cr0, eax
        jmp 0x0:.unreal

    .unreal:
        pop ds          ; Restore data degment
        pop bx
        pop eax
        ret

    gdt_data:      
            ; NULL descriptor
            dq 0

            ; 32-bit code segment
            dw 0xFFFF               ; limit (lower 16 bits) = 0xFFFFF for full 32 bit range
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23) = 0x0
            db 10011010b            ; access (present, ring 0, code segment, executable, direction 0, readable)
            db 11001111b            ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
            db 0                    ; base high

            ; 32-bit data segment
            dw 0xFFFF               ; limit (lower 16 bits) = 0xFFFFF for full 32 bit range
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23) = 0x0
            db 10010010b            ; access (present, ring 0, data segment, executable, direction 0, writeable)
            db 11001111b            ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
            db 0                    ; base high

            ; 16-bit code segment
            dw 0xFFFF               ; limit (lower 16 bits) = 0xFFFFF
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23) = 0x0
            db 10011010b            ; access (present, ring 0, code segment, executable, direction 0, readable)
            db 00001111b            ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
            db 0                    ; base high

            ; 32-bit data segment
            dw 0x0FFFF              ; limit (lower 16 bits) = 0xFFFFF
            dw 0                    ; base (bits 0-15) = 0x0
            db 0                    ; base (bits 16-23) = 0x0
            db 10010010b            ; access (present, ring 0, data segment, executable, direction 0, writeable)
            db 00001111b            ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
            db 0                    ; base high

    gdt_info:  
            dw gdt_info - gdt_data - 1    ; limit = size of GDT
            dd gdt_data
    gdt_end:
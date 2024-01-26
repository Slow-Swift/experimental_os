bits 16

%include "structs.inc"

; Character Codes
%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

section .entry
    global start
    start:
        ; Setup segments all to 0
        xor ax, ax
        mov ss, ax
        mov ds, ax
        mov es, ax

        ; Setup stack pointer
        xor esp, esp
        mov sp, 0x7C00

        ; Save disk
        mov [drive_code], dl

        ; Print setup message
        mov si, setup_msg
        call puts

        ; Ensure disk extensions are present
        call check_extensions_present

        ; Read Partition Header
        xor eax, eax
        inc eax             ; LBA 1
        mov cx, ax          ; Read 1 sector
        mov bx, disk_buffer ; Store results in the disk buffer
        call read_sectors

        ; Read ELF header
        mov eax, [disk_buffer + partition_table_header.usable_start]    ; Get the LBA of the first usable block
        ;mov cx, 1                                                      ; cx should be 1 already
        call read_sectors                                               ; Read the ELF Header
        mov ebx, eax                                                    ; Save LBA of the first usable block in ecx

        ; Check ELF Magic Number
        cmp dword [disk_buffer + elf_header.magic], 0x464C457F    ; Check that the elf magic is correct
        je .ok
            ; * More checks could be added in here but we are limited to a single sector of code
            mov al, 0x33
            call error
    .ok:
        ; * Make sure we have read all the program headers
        xor eax, eax
        mov ax, [disk_buffer + elf_header.prog_table_entry_size]       ; Determine size of a program entry
        mov cx, [disk_buffer + elf_header.prog_table_entry_count]      ; Get number of program entries
        push eax                                                        ; Save information for later
        push ecx                                                        
        mul cx                                                          ; Determine size of program entry table
                                                                        ; Need to do this before loading into dx because multiply clears dx
        mov edx, [disk_buffer + elf_header.prog_table_pos]              ; Get start of program table
        push edx     
        add eax, edx                                                    ; Determine offset of the end of a program entry
        add eax, 511 
        mov ecx, 512                                                    ; Determine size in sectors required (round up)
        xor edx, edx                                                    ; EDX is part of dividend
        div ecx
        
        mov cx, ax          ; Read required sectors
        mov eax, ebx        ; Restore ELF file LBA
        mov bx, multi_sector_buffer ; Store results in the multi sector buffer
        mov dl, [drive_code]
        call read_sectors

        pop esi                         ; esi = program table offset 
        add esi, multi_sector_buffer    ; esi = data start + program_table_offset
        mov ebx, eax                    ; ebx = elf file lba
        pop eax                         ; eax = program table entry count
        jz .read_program_end
        pop ecx                         ; ecx = program table entry size
    .read_program_loop:
        call read_program_header
        add esi, ecx                    ; esi += program table entry size
        dec eax
        jg .read_program_loop
    .read_program_end:
        mov dl, [drive_code]            ; Pass drive to stage 2
        xor ax, ax                      ; push segment 0
        push ax
        push word [multi_sector_buffer + elf_header.entry_pos]  ; push stage 2 address
        retf      

        jmp halt    ; Should neve happen

    ; 
    ; Parameters:
    ;   ebx: lba of elf file
    ;   si : start of program header
    ;
    read_program_header:
        pushad

        ; Check program type is load
        cmp byte [si + elf_program_header.type], 0x01   ; Load type is 0x01
        jne .done   ; If not load type then skip to end
    .type_load:
        mov edi, [si + elf_program_header.virtual_addr] ; Address to load program to 
        mov ecx, [si + elf_program_header.memory_size]  ; Size of program in bytes
        
        ; Initialize memory [edi to edi + ecx] to zero
        xor al, al       ; Byte to write               
        rep stosb
    
        mov eax, [si + elf_program_header.offset]       ; Determine how far down the file the program is
        mov edi, [si + elf_program_header.virtual_addr] ; Reload virtual address since edi has been modified
        mov ecx, 512                                    ; Sector Size
        xor edx, edx                                    ; Clear edx
        div ecx         ; Determine sectors (eax) and offset in sector (edx): offset / sector size, offset % sector size
        push edx    ; esi = offset in sector
        
        add eax, ebx   ; Add the lba of the start of the elf file

        mov cx, 1               ; Read 1 sector
        mov bx, disk_buffer     ; Read into the disk buffer
        mov dl, [drive_code]    ; Read from the drive
        call read_sectors       ; Perform the read

        mov ebx, [si + elf_program_header.file_size]    ; Determine filesize
        pop esi
        
        mov ecx, esi    ; toRead = offset in sector
        add ecx, ebx    ; toRead += remainingToRead
        mov edx, 512
        cmp ecx, edx    ; ecx = min(ecx, 512)
        jle .next
        mov ecx, edx
    .next:
        sub ecx, esi            ; toRead -= offset in sector
        mov edx, ecx            ; save toRead
        add esi, disk_buffer    ; readPos = readOffset + disk_buffer
        rep movsb               ; move ecx bytes from ds:[si] to es:[di]
        sub ebx, edx            ; remainingToRead -= toRead
        jle .done          ; If remainingToRead <= 0 then exit
    .read_loop:
        push ebx                ; Save remaining to read

        ; Read next sector
        inc eax                 ; increment read LBA
        mov bx, disk_buffer     ; read to disk_buffer
        mov cx, 1               ; read 1 sector
        mov dl, [drive_code]    ; read from the boot disk
        call read_sectors       ; perform the read

        pop ebx         ; Restore remainingToRead
        mov ecx, ebx    ; toRead = max(remainingToRead, 512)
        cmp ecx, 512
        jle .after_max
        mov ecx, 512
    .after_max:
        mov edx, ecx
        rep movsb       ; move ecx bytes from ds:[si] to es:[di]
        sub ebx, edx    ; remainingToRead -= toRead
        jg .read_loop

    .done:
        popad
        ret

    ;
    ; Check whether disk extensions are present
    ; If disk extensions are not present the program is halted
    ; Parameters:
    ;   dl: drive code
    ;
    ; Volatile:
    ;   ax, bx, cx, dh
    ;
    check_extensions_present:
        ; Call interrupt 0x13:41 (bx:0x55aa)
        mov ah, 0x41
        mov bx, 0x55AA
        int 0x13
        jc .error       ; Error if the carry flag was set
        ret

    .error:
        mov al, 0x31       ; Error 1: Disk extensions not present
        call error

    ;
    ; Read sectors from the disk
    ; Parameters:
    ;   eax: LBA address of first sector
    ;   cx: Number of sectors to read (max 127)
    ;   es:bx: Memory address to read to
    ;   dl: drive code
    ;
    ; Volatile:
    ;   dap
    ;
    read_sectors:
        push si
        push eax

        ; Setup the disk address packet
        mov [dap.lba], eax
        mov [dap.segment], es
        mov [dap.offset], bx
        mov [dap.count], cx
        
        ; Call the disk read interrupt
        mov si, dap     ; ds:si = disk address packet
        mov ah, 0x42
        int 0x13

        ; TODO: Ensure sectors read matches desired sectors

        jc .error

        pop eax
        pop si
        ret

        .error:
        mov al, 0x32        ; Error 0x32 (2): Error reading from disk
        call error


    ;
    ; Print an error message and halt.
    ; Does not return.
    ; Parameters:
    ;   al: ASCII error code
    ;
    error:
        mov si, error_msg   ; Print the error message
        call puts

        mov ah, 0x0E        ; Print the code
        int 0x10
        jmp halt            ; Halt the program

    ; 
    ; print a string to the screen
    ; Parameters:
    ;   si: address of string
    ;
    puts:
        ; Save modified registers
        push si
        push ax
        
    .loop:
        lodsb       ; Load next character into al
        or al, al   ; Exit if it is null
        jz .done

        mov ah, 0x0E    ; Call interrupt 0x10 - 0x0E to print al
        int 0x10
        jmp .loop       ; Continue to next character
        
    .done:
        ; Restore modified registers
        pop ax
        pop si
        ret

    ;
    ; Halts the program with an infite loop
    ; Never returns
    ;
    halt:
        jmp halt

section .data
    align 4
    dap: 
        .size:      db 0x10     ; Size of dap in bytes
                    db 0        ; Always 0
        .count:     dw 0        ; Sectors to read
        .offset:    dw 0        ; Buffer offset
        .segment:   dw 0        ; Buffer segment
        .lba:       dq 0        ; LBA

    drive_code: db 0

section .rodata
    setup_msg: db "Stage 1", ENDL, 0
    error_msg: db "E: ", 0

section .bss
    disk_buffer: resb 512
    multi_sector_buffer: resb 512

bits 16

extern hard_error
extern put_hex

global disk_initialize
global disk_seek_abs
global disk_seek_cur
global disk_copy_bytes

section .text

    ;
    ; Initialize disk
    ; Parameters:
    ; dl: drive code
    ;
    disk_initialize:
        mov [drive_code], dl
        ret

    ;
    ; Seek to a position in the disk offset from an LBA
    ; Parameters:
    ;   eax: lba start
    ;   ebx: offset from lba
    ;
    disk_seek_abs:
        push eax
        push ebx
        push edx
        
        push eax            ;           Save LBA start
        xor edx, edx        ; <edx>     Clear edx
        mov eax, ebx        ; <eax>     Copy offset to eax for division
        mov ebx, 512        ; <ebx>     Will divide by sector size
        div ebx             ; <eax, edx>    eax = offset sectors, edx = offset in sector
        pop ebx             ; <ebx>     ebx = LBA start
        add eax, ebx        ; <eax>     eax = lba to read
        mov [current_offset], edx   ; Save offset

        call read_sector    ;           Read the sector if it has not been read

        pop edx
        pop ebx
        pop eax
        ret

    ; Seek to a position relative to current position
    ; Parameters:
    ;   ebx: offset
    ;
    disk_seek_cur:
        push eax
        push ebx

        ; Determine absolute offset
        mov eax, [dap.lba]          ; <eax> Read current lba
        add ebx, [current_offset]   ; <ebx> Add the current offset to the seek offset

        call disk_seek_abs   ; Seek to the absolute offset 

        pop ebx
        pop eax
        ret

    ;
    ; Parameters:
    ;   es:di: address to copy to
    ;   ebx: bytes to copy
    ;
    ; Modifies:
    ;   es:di: points to address after last copied byte
    ;
    disk_copy_bytes:
        push eax
        push ebx
        push ecx
        push edx
        push di
        push si

        xor edx, edx                ; <edx>     clear edx
        mov ds, edx                 ; <ds>      clear data segment
        mov eax, [dap.lba]          ; <eax>     Save current LBA

    .read_loop:
        mov ecx, ebx                ; <ecx>     ecx = bytes to read
        add ecx, [current_offset]   ; <ecx>     Add offset in sector to get total offset
        mov edx, 512                ; <edx>     edx = sector size
        cmp ecx, edx                ;           ecx = min(ecx, 512)
        cmovg ecx, edx              ; <ecx>
        sub ecx, [current_offset]   ; <ecx>     Subtract offset in sector to get sector bytes to read

        mov si, segment_buffer      ; <esi>     si to start of read buffer
        add si, [current_offset]    ; <esi>     Add read offset to si

        sub ebx, ecx                ; <ebx>     Update bytes to read
        add [current_offset], ecx   ;           Update current offset with read bytes
        rep movsb                   ;           Copy the bytes

        ; If entire sector read, advance to next sector
        cmp [current_offset], edx   ;           Check if whole sector read
        jl .after_advance_sector
        inc eax                     ; <eax>     Advance to next sector
        xor edx, edx                ; <edx>     Clear edx
        mov [current_offset], edx   ;           Clear current offset in sector
        call read_sector            ;           Read the new sector
    .after_advance_sector:
        test ebx, 0
        jg .read_loop   
    
        pop si
        pop di
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret

    ;
    ; Read a sector from the disk into the buffer
    ; Parameters:
    ;   eax: LBA address of first sector
    ;
    ; Volatile:
    ;   dap
    ;
    read_sector:
        ; If the LBA has been read into the buffer then nothing need be done
        cmp eax, [dap.lba]  
        jne .perform_read
        ret

    .perform_read:
        push si
        push ds
        push eax
        push ebx

        xor ebx, ebx        ; <ebx>     Clear ebx
        mov ds, ebx         ; <ds>      Clear ds
        inc ebx             ; <ebx>     ebx = 1

        ; Setup the disk address packet
        mov [dap.lba], eax
        mov [dap.count], bx    ;           Read 1 sector
        mov word [dap.offset], segment_buffer
        mov dl, [drive_code]    ; <edx>     Copy drive code to dl
        
        ; Call the disk read interrupt
        mov si, dap     ; <esi>     ds:si = disk address packet
        mov ah, 0x42    ; <eax>
        sti
        int 0x13
        cli

        jc .error       ; Carry should be set on an error

        ; Ensure 1 sector has been read
        cmp [dap.count], bx
        jne .error

        pop ebx
        pop eax
        pop ds
        pop si
        ret

        .error:
            mov si, read_error
            call hard_error

section .data
    align 4
    dap: 
        .size:      db 0x10     ; Size of dap in bytes
                    db 0        ; Always 0
        .count:     dw 0        ; Sectors to read
        .offset:    dw segment_buffer        ; Buffer offset
        .segment:   dw 0        ; Buffer segment
        .lba:       dq 0        ; LBA

    drive_code: db 0
    current_offset: dd 0

section .rodata:
    read_error: db "Disk Error: Could not read sector", 0

section .bss
    segment_buffer: resb 512
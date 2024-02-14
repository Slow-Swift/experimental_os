bits 16

%include "common.inc"
%include "elf_structs.inc"

extern puts
extern put_hex
extern putline
extern hard_error

extern fat_open_root_directory
extern fat_find_file_in_directory
extern fat_open_file
extern fat_copy_bytes
extern fat_seek_bytes

global kl_load_kernel
global load_program

section .text

    ;
    ; Load the kernel
    ; Will search for the kernel in "BOOT/KERNEL.ELF"
    ; Return:
    ;   eax: address of kernel entry
    ;
    kl_load_kernel:
        push ebx
        push edi
        push esi

        call open_kernel
        mov edi, elf_header
        mov ebx, elf_header_struct_size
        call fat_copy_bytes

        cmp dword [elf_header + elf_header_struct.magic], 0x464C457F    ; Check that the elf magic is correct
        jnz .invalid_header

        mov si, kernel_header_valid_msg
        call puts

        mov si, loading_kernel_msg
        call puts

        call read_program_headers

        mov si, kernel_loaded_msg
        call puts

        mov eax, [elf_header + elf_header_struct.entry_pos] ; <eax>     Load the address of the kernel entry

        pop esi
        pop edi
        pop ebx
        ret
    .invalid_header:
        mov si, invalid_header_err
        call hard_error

    open_kernel:
        push eax
        push ebx
        push si

        call fat_open_root_directory        ;           Open the root directory
        mov si, boot_folder                 ; <esi>     Will look for boot directory
        call fat_find_file_in_directory     ; <eax, ebx>    Find the boot directory

        test eax, eax                       ;           Check cluster not zero (file not found)
        je .boot_folder_not_found           ;           If cluster zero then file was not found
        not bl                              ; <ebx>     If bl is not -1 then it was not directory
        test bl, bl
        jnz .boot_folder_not_found          ;           Error if it was not a directory

        not bl
        call fat_open_file                  ;           Open the root directory
        mov si, kernel_name                 ; <esi>     Search for the kernel name
        call fat_find_file_in_directory     ; <eax, ebx>    Find the kernel file
        test eax, eax                       ;           Check cluster not zero (file not found)
        je .kernel_not_found                ;           If cluster zero then error since file was not found

        mov si, kernel_found_msg
        call puts

        call fat_open_file      ; Open the kernel file

        pop si
        pop ebx
        pop eax
        ret
    .boot_folder_not_found:
        mov si, boot_folder_not_found_err
        call hard_error
    .kernel_not_found:
        mov si, kernel_not_found_err
        call hard_error

    read_program_headers:
        push eax
        push ebx
        push ecx
        push edi

        mov cx, [elf_header + elf_header_struct.prog_table_entry_count]    ; <ebx>     Determine number of headers

        mov eax, [elf_header + elf_header_struct.prog_table_pos]  ; <eax>     Determine position of header table
    .read_loop:
        call fat_seek_bytes                         ; Seek to next program header
        mov ebx, elf_program_header_struct_size     ; <ebx> Bytes to copy
        mov edi, prog_header                        ; <edi> Location to copy to
        call fat_copy_bytes                         ;       Copy the program header
        call load_program

        add eax, [elf_header + elf_header_struct.prog_table_entry_size] ; <eax>     Advance eax to next program header
        dec cx          ; <ecx> Calculate remaining program heaaders
        jne .read_loop  ;       If there are more then read them
    .done:
        pop edi
        pop ecx
        pop ebx
        pop eax
        ret

    load_program:
        push eax
        push ebx
        push ecx
        push edi
        push si

        mov si, reading_program_msg
        call puts

        mov eax, [prog_header + elf_program_header_struct.virtual_addr] ; <eax>     Address to clear from
        mov edi, eax    ; <edi>     edi = address to load to

        ; Print virtual address
        mov si, mem_address_msg 
        call puts
        call put_hex
        call putline

        mov eax, [prog_header + elf_program_header_struct.memory_size]  ; <eax>     Size of memory to clear
        mov ecx, eax        ; <ecx>     Size of memory to clear

        ; Print memory size
        mov si, mem_size_msg
        call puts
        call put_hex
        call putline

        push edi            ; Save address
        test ecx, ecx       ;   Check if memory must be cleared
        jz .clear_mem_done  ;   If nothing to clear then skip

        push ecx            ; Save ecx
        add ecx, edi        ; <ecx> Add memaddr to memsize
        cmp ecx, edi        ;       Check if memory will wrap around
        jle .mem_too_large  ;       If it will then error
        pop ecx             ; <ecx> Restore size to clear

        xor ax, ax          ; <eax> Clear ax to write memory
        mov es, ax          ; <ds>  Clear segment register

    .clear_mem_loop:
        mov [es:edi], al    ;       Clear byte at eax
        inc edi             ; <edi> Move to next byte
        dec ecx             ; <ecx> Decrement bytes to clear
        jg .clear_mem_loop  ;       If more to clear then do so
    .clear_mem_done:
        mov eax, [prog_header + elf_program_header_struct.file_size]    ; <eax> Size of bytes to copy
        mov ebx, eax    ; <ebx> Size of bytes to copy
        mov esi, file_size_msg 
        call puts
        call put_hex
        call putline

        mov eax, [prog_header + elf_program_header_struct.offset]   ; <eax> Offset of code in file
        call fat_seek_bytes     ; Seek to the code

        pop edi     ; <eax> Address to copy to
        call fat_copy_bytes

        pop si
        pop edi
        pop ecx
        pop ebx
        pop eax
        ret

    .mem_too_large:
        mov si, mem_too_large_error
        call hard_error

section .rodata
    boot_folder_not_found_err:  db "KERNEL_LOAD_ERROR: Could not find boot folder", 0
    kernel_not_found_err:  db "KERNEL_LOAD_ERROR: Could not find kernel file", 0
    invalid_header_err: db "KERNEL_LOAD_ERROR: Invalid ELF Header", 0
    mem_too_large_error: db "KERNEL_LOAD_ERROR: Too much memory required", 0
    boot_folder: db "BOOT       ", 0
    kernel_name: db "KERNEL  ELF", 0
    kernel_found_msg: db "Found Kernel", ENDL, 0
    kernel_header_valid_msg: db "Kernel Header Valid", ENDL, 0
    kernel_loaded_msg:      db "Loaded Kernel", ENDL, 0
    loading_kernel_msg:     db "Loading Kernel", ENDL, 0
    reading_program_msg:    db "  Loading Program", ENDL, 0
    mem_address_msg:        db "    Memory Address: 0x", 0
    mem_size_msg:           db "    Memory Size:    0x", 0
    file_size_msg:          db "    File Size:      0x", 0

section .bss
    elf_header: resb elf_header_struct_size
    prog_header: resb elf_program_header_struct_size
    sector_buffer: resb SECTOR_SIZE
bits 16

%include "gpt_structs.inc"
%include "fatstructs.inc"

%define SECTOR_SIZE 512

extern gpt_get_partition
extern disk_seek_abs
extern disk_copy_bytes
extern disk_read_sector

extern put_hex
extern putline
extern hard_error

extern puts
extern putline

global fat_initialize
global fat_open_root_directory
global fat_open_file
global fat_advance_sector
global fat_find_file_in_directory
global fat_copy_sector
global fat_copy_bytes
global fat_seek_bytes
global fat_seek_sector

section .text

    fat_initialize:
        push eax
        push ebx
        push di

        xor ebx, ebx        ; <ebx>     Start with partition 0
        mov di, partition   ; <edi>     Address to load partition to
        
    ; While a usable parition was not found, try to find one
    .get_partition_loop:
        mov eax, ebx            ; <eax> Copy parition number to eax
        call gpt_get_partition  ;       Get the parition
        test ax, ax             ;       Check if return value is nonzero
        jnz .partition_found    ;       If a used parition was found then exit
        inc ebx                 ; <ebx> Check next partition

    .partition_found:
        ; Seek to the start of the partition
        mov eax, [partition + partition_entry.lba_start]    ; <eax>     Load LBA of parition start
        mov ebx, 0                                          ; <ebx>     Want to seek to sector start
        call disk_seek_abs                                  ;           Perform the seek

        ; Copy the bios parameter block
        mov ebx, ebpb_struct_size           ; <ebx>     Number of bytes to copy
        mov di, e_bios_parameter_block      ; <edi>     Address to copy to
        call disk_copy_bytes                ;           Perform the copy

        ; Determine lba of data section
        xor ebx, ebx    ; <ebx>     Clear ebx
        mov eax, [e_bios_parameter_block + ebpb_struct.sectors_per_fat] ; <eax>     Copy sectors per fat
        mov bl, [e_bios_parameter_block + ebpb_struct.fat_count]        ; <ebx>     Copy fat count
        mul ebx                                                         ; <edx, eax>    edx:eax = sectors_per_fat * fat_count
        xor ebx, ebx
        mov bx, [e_bios_parameter_block + ebpb_struct.reserved_sectors] ; <ebx>     Copy reserved sector count
        add eax, ebx                                                    ; <eax>     eax = sectors_per_fat * fat_count + reserved_sectors
        
        ;* Note: This only works if eax + partition_entry.lba_start is a four byte number
        ;*       partition_entry.lba_start is an 8 byte number
        ;*       As long as the data lba is within 4GB of the start of the disk, then we are fine
        ;*       If this is not the case a lot of other things need to change too.
        add eax, [partition + partition_entry.lba_start]                 ; <eax>     eax += partition start
        mov [data_section_lba], eax     ; Save lba of the data section

        xor eax, eax                                                        ; <eax> Clear fat sector that will be loaded
        mov [current_fat_sector], eax                                       ; <eax> Update the data
        mov ax, [e_bios_parameter_block + ebpb_struct.reserved_sectors]    ; <eax> Add the number of reserved sectors
        add eax, [partition + partition_entry.lba_start]                    ; <eax> Add the parition offset
        mov di, fat_sector_buffer
        call disk_read_sector

        pop di
        pop ebx
        pop eax
        ret

    ;
    ; Parameters:
    ;   si: filename
    ;
    fat_find_file_in_directory:
        push cx
        push di
        push si

        mov bl, [is_directory]  ; <ebx>     Check if the open file is a directory
        not bl                  ; <ebx>     Flip the bits. Result is 0 if a directory
        jne .file_not_found     ;           If not a directory then we couldn't find the file

        ; Seek to the start of the directory
        xor eax, eax            ; <eax>     Seek to sector 0
        call fat_seek_sector    ;           Perform the seek

    .check_dir_entry:
        mov di, [current_offset_in_sector]                          ; <di>     Directory entry pointer = current offset
        add di, file_sector_buffer + dir_entry_struct.name          ; <di>     Update the pointer to the start of the file name
        cmp byte [di], 0        ;           Check if the first byte is null
        je .file_not_found      ;           If so, there are no more files so the file was not found
        push di                 ;           Save directory entry name address
        push si                 ;           Save filename address
        mov cx, 11              ; <ecx>     Number of characters to check
        repe cmpsb              ; <eds, esi, ecx>   Compare 11 chars or until a mismatch is found
        pop si                  ; <esi>     Restore filename address
        pop di                  ; <edi>     Restore directory entry name address
        test cx, cx             ;           Check if cx is 0
        je .file_found
        mov eax, dir_entry_struct_size  ; <eax>     Will seek to the next dir entry
        call fat_seek_rel               ;           Perform the seek
        ;TODO: Check seek ok
        jmp .check_dir_entry
    .file_found:
        mov ax, [di+dir_entry_struct.first_cluster_high] ; <eax>     Copy upper two bytes of cluster to ax
        shl ax, 16                                       ; <eax>     Shift the upper to bytes of cluster to top of eax
        mov ax, [di+dir_entry_struct.first_cluster_low]  ; <eax>     Copy lower two bytes of cluster to bottom of eax
        xor bl, bl                                       ; <ebx>     Assume not a directory
        mov cl, [di+dir_entry_struct.attributes]         ; <ecx>     Check the attributes
        test cl, 0x10                                    ;           Check if it is a directory
        je .done
        not bl              ; <ebx>     The file is a directory so mark it as such
        jmp .done
    .file_not_found:
        xor eax, eax        ; <eax> Cluster 0 means it is not valie
        xor bl, bl          ; <ebx> We don't want to indicate that this is a directoy since it is invalid
    .done:
        pop si
        pop di
        pop cx
        ret

    ;
    ; Seek a number of bytes forward
    ; Parameters:
    ;   eax: number of bytes to seek forward
    ;
    fat_seek_rel:
        push eax
        push ebx
        push edx

        xor ebx, ebx                        ; <ebx>     Clear ebx
        mov bx, [current_offset_in_sector]  ; <ebx>     ebx = offset in sector
        add eax, ebx                        ; <eax>     eax = target offset from sector start
        mov ebx, SECTOR_SIZE                ; <ebx>     divide by size of sector
        xor edx, edx                        ; <edx>     Clear upper bytes of dividend
        div ebx                             ; <eax, edx>    eax = sector offset, edx = offset in sector
        push eax                            ;           Save sector offset
        push dx                             ;           Save offset in sector
        mov eax, [current_cluster_index]                                        ; <eax> = current cluster index
        mov ebx, [e_bios_parameter_block + ebpb_struct.sectors_per_cluster]     ; <ebx> = sectors per cluster
        mul ebx                                                                 ; <edx:eax> = current sector index
        pop dx                              ; <edx>   Restore offset in sector
        pop ebx                             ; <ebx> = sector offset
        add eax, ebx                        ; <eax> = target sector index
        call fat_seek_sector                ;         Seek to the sector
        mov [current_offset_in_sector], dx  ;         Save the offset in the sector

        pop edx
        pop ebx
        pop eax
        ret

    ;
    ; Seek to a byte position in the file
    ; eax: position in the file
    ;
    fat_seek_bytes:
        push eax
        push ebx
        push edx

        xor edx, edx            ; <edx>     Clear upper bytes of dividend
        xor ebx, ebx            ; <ebx>     Clear divisor
        mov bx, SECTOR_SIZE     ; <ebx>     divisor = sector size
        div ebx                 ; <eax, edx>    eax = sector, edx = offset in sector

        call fat_seek_sector    ;           Seek to the sector
        mov [current_offset_in_sector], dx  ; Update position in sector

        pop edx
        pop ebx
        pop eax
        ret

    ; 
    ; Seek to a sector from the start of the file
    ; Parameters:
    ;   eax: sector index
    fat_seek_sector:
        push eax
        push ebx
        push ecx
        push edx

        ; Determine cluster index and sector offset to seek to
        xor edx, edx    ; <edx>     Clear high bits of dividend
        xor ebx, ebx
        mov bl, [e_bios_parameter_block + ebpb_struct.sectors_per_cluster] ; <ebx>     Divide sector by sectors per cluster
        div ebx         ; <eax, edx>    eax = cluster index, edx = sector in cluster

    .seek_sector:
        ; Check if seeking backwards
        mov ecx, eax                            ; <ecx>     ecx = target cluster index
        mov ebx, [current_cluster_index]        ; <ebx>     ebx = current cluster index
        cmp ecx, ebx                            ;           Check if current cluster is target
        jne .must_seek                          ;           If it is not seeking must be done
        cmp dl, [current_sector_in_cluster]     ;           Check if current sector in cluster is target
        jne .must_seek                          ;           If it is not seeking must be done
        xor dx, dx
        mov [current_offset_in_sector], dx
        jmp .done
    
    .must_seek:
        mov eax, [current_cluster]              ; <eax>     eax = current cluster
        cmp ecx, ebx                            ;           Check if target cluster is before the current one
        jge .seek_cluster

        ; If seeking a cluster further back in the file then reset to first cluster
        mov eax, [first_cluster]    ; <eax>     Reset eax to first cluster
        xor ebx, ebx                ; <ebx>     Reset cluster index to first cluster
    
    ; Get the cluster for the cluster index
    .seek_cluster:
        cmp ecx, ebx
        jle .cluster_found          ;           If the target cluster index is equal (less should never happen) then we have found the cluster
        call get_next_cluster       ; <eax>     Get the next cluster
        cmp eax, 0x0FFF_FFF7        ;           Anything greater than this is not a valid cluster
        jge .error                  ;           If not a valid sector then throw an error
        inc ebx                     ; <ebx>     Increment the cluster index
        jmp .seek_cluster
    .cluster_found:
        ; Save seeking information
        mov [current_cluster], eax              ; Save current cluster
        mov [current_sector_in_cluster], dl     ; Save cluster sector offset
        mov [current_cluster_index], ebx        ; Save current cluster index
        xor dx, dx                              ; <edx>   Clear dx
        mov [current_offset_in_sector], dx      ; Clear offset in sector

        ; Load the correct sector
        call load_sector                        ; Load the new sector

    .done:
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret
    .error: 
        mov si, error_invalid_sector
        call hard_error

    ;
    ; Copy the current sector and advance to the next
    ; Paramters:
    ;   es:edi: address to copy to
    ;   ebx: bytes to copy
    ;
    fat_copy_bytes:
        push eax
        push ebx
        push ecx
        push edx
        push edi
        push esi

        mov dx, SECTOR_SIZE             ; <edx>     Save the size of a sector
        
        xor ecx, ecx                    ; <ecx>     Clear ecx
        mov cx,  bx                     ; <ecx>     Store bytes to read in cx
        add cx, [current_offset_in_sector]  ; <ecx>     Add the offset in sector

        cmp cx, dx                          ; <ecx>     If need to read past the end of a sector
        cmovg cx, dx                        ; <ecx>     Limit to the end of the sector
        sub cx, [current_offset_in_sector]  ; <ecx>     Subtract offset in sector to get bytes to read

    .read_loop:
        sub ebx, ecx                        ; <ebx>     Subtract bytes read from bytes to read
        xor esi, esi
        mov si, file_sector_buffer          ; <esi>     Read from file sector buffer
        add si, [current_offset_in_sector]  ; <esi>     Add offset in sector
    .copy_loop:
        add [current_offset_in_sector], ecx ; Update the offset in the sector
        mov al, [si]        ; <eax> al = byte to copy
        mov [es:edi], al    ;       Copy byte from al to destination
        inc edi             ; <edi> Move to next destination byte
        inc si              ; <esi> Move to next source byte
        dec cx              ; <ecx> Decrement bytes to move
        jnz .copy_loop      ;       Copy next byte if there is more
    
        cmp dx, [current_offset_in_sector]  ; Check if entire sector read
        jg .after_advance_sector                       
        call fat_advance_sector             ; If entire sector read then move to the next one

    .after_advance_sector:
        test ebx, ebx       ;           Check if more bytes to read
        jz .read_done       ;           If not then exit
        mov cx, bx          ; <ecx>     cx = bytes to read
        cmp cx, dx          ;           Check if reading more than a sector
        cmovg cx, dx        ; <ecx>     If so then limit to a single sector
        jmp .read_loop

    .read_done:
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret

    ;
    ; Copy the current sector and advance to the next
    ; Paramters:
    ;   di: address to copy to
    ;
    fat_copy_sector:
        push cx
        push di
        push si
        
        mov cx, SECTOR_SIZE
        mov si, file_sector_buffer
        rep movsb

        call fat_advance_sector

        pop si
        pop di
        pop cx

    ;
    ; Open the root directory
    ;
    fat_open_root_directory:
        mov eax, [e_bios_parameter_block + ebpb_struct.root_cluster]    ; First cluster is the root cluster

        ; ebx = -1 since it is a directory
        xor ebx, ebx
        not ebx

        ; Open the file
        call fat_open_file
        ret

    ;
    ;
    ; Parameters:
    ;   eax: first cluster of file
    ;   bl: -1 if is directory, else 0
    fat_open_file:
        push eax
        push ebx
        push edx

        mov [first_cluster], eax
        mov [current_cluster], eax
        mov [is_directory], bl

        xor edx, edx                        ; <edx>     Clear edx
        mov [current_offset_in_sector], dx ;           Current offset in sector = 0
        mov [current_cluster_index], edx    ;           Current cluster index = 0
        mov [current_sector_in_cluster], dl     ;       Current sector in cluster = 0

        call load_sector    ; Load the sector into the buffer

        pop edx
        pop ebx
        pop eax
        ret

    ; 
    ; Advance to the next sector
    ; Returns:
    ;   eax 0 if success, -1 otherwise
    fat_advance_sector:
        push ebx
        xor bx, bx

        ; Advance sector in cluster
        mov [current_offset_in_sector], bx      ;           Reset current offset in sector
        mov bl, [current_sector_in_cluster]     ; <ebx>     Load current sector in cluster
        inc bl                                  ; <ebx>     Move to next sector in cluster

        ; Check if we need to move to the next cluster
        cmp bl, [e_bios_parameter_block + ebpb_struct.sectors_per_cluster] 
        jge .advance_next_cluster               ; Advance to the next cluster if we need to

        ; If we don't then just load the sector
        mov [current_sector_in_cluster], bl     ; Store the new sector in cluster
        call load_sector                        ; Load the new sector
        pop bx
        ret

    .advance_next_cluster:
        ; Get the next cluster
        xor bl, bl                  ; <ebx>     Current sector in cluster = 0
        mov eax, [current_cluster]  ; <eax>     Get the current cluster
        call get_next_cluster       ; <eax>     Get the next cluster

        ; Ensure it is a valid sector
        cmp eax, 0x0FFF_FFF7        ; Anything greater than this is not a valid cluster
        jl .valid_cluster
        xor eax, eax        ; <eax>     Set eax to -1 if it was an invalid sector
        not eax
        jmp .done
    .valid_cluster:
        ; We have found a valid cluster
        mov [current_cluster], eax          ; Store the new cluster number
        ; Update the cluster index
        mov eax, current_cluster_index      ; <eax>     Get current cluster 
        inc eax                             ; <eax>     Advance to the next cluster index
        mov [current_cluster_index], eax    ;           Store current cluster index
        mov [current_sector_in_cluster], bl ;           Store sector in cluster (0)
        call load_sector                    ;           Load the sector
        xor eax, eax
    .done:
        pop ebx
        ret

    ;
    ; Get the next cluster in the chain
    ; Parameters:
    ;   eax: current cluster
    ; Returns:
    ;   eax: The next cluster
    ;
    get_next_cluster:
        push ecx
        push edx
        push di

        ; Current cluster *= 4      (each cluster uses 32bits or four bytes in the FAT)
        shl eax, 1      ; <eax>     Current Cluster *= 2
        shl eax, 1      ; <eax>     Current Cluster *= 2

        ; Determine sector in the FAT that we need to read from
        xor edx, edx            ; <edx>     Clear edx, upper bytes of dividend
        mov ecx, SECTOR_SIZE    ; <ecx>     Will divide by the sector size
        div ecx                 ;           EAX = sector, EDX = offset in sector

        cmp eax, [current_fat_sector]   ; Check if the sector has already been loaded
        je .fat_sector_loaded

        ; Sector has not been loaded so load it
        mov [current_fat_sector], eax
        xor ebx, ebx
        mov bx, [e_bios_parameter_block + ebpb_struct.reserved_sectors]
        add eax, ebx    ; <eax> Add the number of reserved sectors
        add eax, [partition + partition_entry.lba_start]                    ; <eax> Add the parition offset
        mov di, fat_sector_buffer
        call disk_read_sector

    .fat_sector_loaded:
        mov eax, [fat_sector_buffer + edx]  ; <eax> Read the value of the next cluster
        and eax, 0x0FFF_FFFF                ; <eax> Ignore the upper four bits

        pop di
        pop edx
        pop ecx
        ret

    ;
    ; Read the current sector into the buffer
    ;
    load_sector:
        push eax
        push ebx
        push edx
        push di

        mov eax, [current_cluster]

        ; Cluster -= 2 (not sure why we need this but osdev wiki says we do)
        dec eax
        dec eax

        xor ebx, ebx    ; <ebx>     Clear ebx
        mov bl, [e_bios_parameter_block + ebpb_struct.sectors_per_cluster]  ; <ebx>     Load sectors per cluster
        mul ebx                     ; <edx, eax>    eax (sector) = cluster * sectors per cluster
        add eax, [data_section_lba] ; <eax>         eax (sector) += data section start
        xor ebx, ebx                            ; Clear ebx
        mov bl, [current_sector_in_cluster]     ; <ebx>     Get sector in cluster
        add eax, ebx                ; <eax>     Add offset of sector in cluster
        mov di, file_sector_buffer  ; <edi>     Address to copy the sector to
        call disk_read_sector

        pop di
        pop edx
        pop ebx
        pop eax
        ret

section .data
global dbg_fat_data
    dbg_fat_data:
    data_section_lba:   dd 0
    current_fat_sector: dd 0
    first_cluster:      dd 0
    current_cluster:    dd 0
    current_cluster_index:  dd 0
    current_offset_in_sector:   dw 0
    current_sector_in_cluster:  db 0
    is_directory:       db 0

section .rodata
    error_invalid_sector: db "FAT ERROR: Seek Sector Invalid", 0

section .bss
    partition: resb partition_entry_size
    e_bios_parameter_block: resb ebpb_struct_size
    file_sector_buffer: resb SECTOR_SIZE
    global dbg_fat_buff
    dbg_fat_buff:
    fat_sector_buffer:  resb SECTOR_SIZE
    lfn_buffer: resb 512    ;   256 2_byte characters
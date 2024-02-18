# Information about how the system works

## Boot Process

There are three stages to this OS:
- Stage 1
- Stage 2
- Kernel

Stage 1 fits in the 512 byte first sector of the disk. It expects to be loaded to memory address 0x7C00 and be passed the disk code in dl by the bios. It also expects the drive to be a GPT drive and for Stage 2 to be an ELF file located starting at the first usable sector of the disk as determined by the GPT header. It parses the ELF file and loads it into memory as determined by the virtual memory address of the ELF program headers (Probably 0x1000). Care should be taken that Stage 2 is not large enough to overwrite stage 2. After loading Stage 2 into memory it ensures that dl contains the drive code and jumps to the entry point of stage 2.

Stage 2 should be loaded to memory address 0x1000 and dl should contain the drive code. Stage 2 will locate and load the kernel as well as collect some information about the systes which will be stored at 0x500. The address of this info will be passed to the kernel. Stage 2 reads the GPT header and uses the first partition as the partition to boot from and expects it to use the FAT 32 format. It performs the following:
 - Enable the A20 gate if it is disabled
 - Initialize a FAT 32 Driver
 - Initialize Disk routines
 - Read the GPT Header and find the boot paritition
 - Initialize FAT 32 routines with the partition
 - Use BIOS interrupt 0x15:E820 to get a memory map
 - Load a GPT with 16 and 32 bit flat memory models
 - Enter Unreal mode
 - Find the kernel file at BOOT/KERNEL.ELF
 - Parse the kernel file and load the code into memory (Should be at 0x100000 (1MiB))
 - Enter 32 bit protected mode and call the kernel start function

The Kernel does nothing at the moment.

    symbol-file /home/lugfini/Documents/Programming/os_expriments/build/i686_debug/kernel/kernel.elf
    set disassembly-flavor intel
    target remote | qemu-system-i386 -S -gdb stdio -m 32 -hda /home/lugfini/Documents/Programming/os_expriments/build/i686_debug/image.img
    set tdesc filename /home/lugfini/Documents/Programming/os_expriments/scripts/target.xml


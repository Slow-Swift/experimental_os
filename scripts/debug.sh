#!/bin/bash

if [ "$#" -le 0 ]; then
    echo "Usage: ./debug.sh <image>"
    exit 1
fi
QEMU_ARGS="-S -gdb stdio -m 32 -hda $PWD/$1"

# b *0x7c00
# layout asm
cat > .vscode/.gdb_script.gdb << EOF
    layout asm
    set disassembly-flavor intel
    target remote | qemu-system-i386 $QEMU_ARGS
    set tdesc filename $PWD/scripts/target.xml
    b *0x7C00
EOF

gdb -x .vscode/.gdb_script.gdb
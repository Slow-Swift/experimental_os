nasm -f bin src/boot.asm -o build/boot.bin

dd if=/dev/zero of=./build/image.dd bs=1048576 count=128
sfdisk ./build/image.dd < build_tools/disk_layout.layout
losetup -o $[2048*512] --sizelimit $[8*1024*1024]
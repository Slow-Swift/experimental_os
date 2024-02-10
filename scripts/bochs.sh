#!/bin/bash

if [ "$#" -le 0 ]; then
    echo "Usage: ./bochs.sh <image>"
    exit 1
fi

DISK_CFG="ata0-master: type=disk, path=\"$1\""
BOOT_CFG="boot: disk"

cat > .bochs_config << EOF
megs: 256
romimage: file=/usr/share/bochs/BIOS-bochs-legacy
vgaromimage: file=/usr/share/bochs/VGABIOS-lgpl-latest
mouse: enabled=0
display_library: x, options="gui_debug"

$DISK_CFG
$BOOT_CFG
EOF

bochs -f .bochs_config
rm -f .bochs_config

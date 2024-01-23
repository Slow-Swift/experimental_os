#!/bin/bash

QEMU_ARGS='-debugcon stdio -m 32'

if [ "$#" -le 0 ]; then
    echo "Usage: ./run.sh <image>"
    exit 1
fi

QEMU_ARGS="${QEMU_ARGS} -hda $1"

echo "Running: qemu-system-x86_64 $QEMU_ARGS"
qemu-system-i386 $QEMU_ARGS

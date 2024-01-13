#!/bin/bash

QEMU_ARGS='-debugcon stdio -m 32'

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <image>"
    exit 1
fi

case "$1" in
    "floppy")   QEMU_ARGS="${QEMU_ARGS} -fda $2"
    ;;
    "disk")     QEMU_ARGS="${QEMU_ARGS} -hda $2"
    ;;
    *)          echo "Unkown image type $1."
                exit 2
esac

echo "Running: qemu-system-i386 $QEMU_ARGS"
qemu-system-i386 $QEMU_ARGS

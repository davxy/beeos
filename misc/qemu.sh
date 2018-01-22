#!/bin/sh

QEMU_X86=qemu-system-i386

# Default command
ARCH="x86"
MEM=12
KERN="../kernel/build/$ARCH/kernel"

while getopts "da:m:k:" opt; do
    case "$opt" in
        k) KERN=$OPTARG ;;
        d) EXTRA="-S -s" ;;
        a) ARCH=$OPTARG ;;
        m) MEM=$OPTARG ;;
    esac
done


case $ARCH in
    "x86")
        QEMU=$QEMU_X86
        ARCH_OPTS=$X86_OPTS
        ;;
    "arm")
        QEMU=$QEMU_ARM
        ARCH_OPTS=$ARM_OPTS
        ;;
esac


if [ "$EUID" = "0" ]; then
    echo "Starting qemu as root"
else
    echo "Starting qemu as user"
fi

echo "memory:" $MEM "MB"
echo "arch:" $ARCH
echo "kernel:" $KERN

EXTRA="$EXTRA -initrd disk.img -serial stdio"

#echo $QEMU -kernel $KERN -m $MEM $ARCH_OPTS $EXTRA

$QEMU -kernel $KERN -m $MEM $ARCH_OPTS $EXTRA &

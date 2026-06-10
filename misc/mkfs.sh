#!/bin/sh

# Abort on the first error: a partially populated image silently
# panics the kernel at boot ("init exiting").
set -e

# Root source
ROOT_SRC=../user/build/x86

# Cleanup possible leftovers from a previous failed run
umount tmp 2>/dev/null || true

# Create the image and make the filesystem.
# The kernel ext2 driver assumes 1024-byte blocks and 128-byte inodes
# (revision 0 layout). Modern e2fsprogs defaults to 256-byte inodes,
# which the driver misreads, so the values are forced here.
dd if=/dev/zero of=disk.img bs=1M count=1
mkfs.ext2 -F -b 1024 -I 128 disk.img

# Mount using a dynamically allocated loopback device.
# /dev/loop0 may already be taken (e.g. by snap squashfs images).
mkdir -p tmp
mount -o loop disk.img tmp

# Copy the sysroot in the destination
cp -r sysroot/* tmp/
mkdir -p tmp/dev
mkdir -p tmp/etc
mkdir -p tmp/home
cp ../README.md tmp/home/README
cp ../TODO tmp/home/TODO

# Create destination directories
DIRS=`find $ROOT_SRC/* -type d | sed "s|$ROOT_SRC|tmp|g"`
mkdir -p $DIRS

# Copy the files
SRC_FILES=`find $ROOT_SRC -perm /a+x -type f`
for f in $SRC_FILES; do
    d=`echo $f | sed "s|$ROOT_SRC|tmp|g"`
    cp $f $d
done

# Umount the destination and release the loopback device
umount tmp
rmdir tmp

echo "disk.img successfully created"

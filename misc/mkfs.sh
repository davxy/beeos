#!/bin/sh

# Root source
ROOT_SRC=../user/build/x86

umount /dev/loop0
losetup -d /dev/loop0

# Create the image and make the filesystem
dd if=/dev/zero of=disk.img bs=1M count=1
mkfs.ext2 disk.img

# Setup loopback device and mount to a temporary directory
losetup -f
sleep 1
losetup /dev/loop0 disk.img
mkdir -p tmp
mount /dev/loop0 tmp

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

sync

# Umount the destination and shutdown the loopback device
#umount /dev/loop0
#losetup -d /dev/loop0

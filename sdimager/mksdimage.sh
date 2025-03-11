#!/bin/bash

MLO=$1
IMG=$2
IMG_SIZE_MB=100
BOOT_PART_SIZE_MB=50
LOOPDEV=""

set -e

if [ -z "$MLO" ]; then
    echo "Usage: $0 <mlo_file> <output_file>"
    exit 1
fi
if [ -z "$IMG" ]; then
    echo "Usage: $0 <mlo_file> <output_file>"
    exit 1
fi

if [ ! -f "$MLO" ]; then
    echo "Error: $MLO not found"
    exit 1
fi

echo "Creating $IMG ($IMG_SIZE_MB MB)..."
dd if=/dev/zero of="$IMG" bs=1M count=$IMG_SIZE_MB status=progress

echo "Partitioning $IMG..."
fdisk "$IMG" <<EOF
o
n
p
1

+${BOOT_PART_SIZE_MB}M
t
c
a
w
EOF

echo "Creating loop device..."
LOOPDEV=$(losetup -f)
sudo losetup $LOOPDEV "$IMG"

sudo partprobe $LOOPDEV

echo "Creating FAT32 filesystem..."
sudo mkfs.vfat -F 32 ${LOOPDEV}p1

echo "Copying MLO..."
if mountpoint -q ./tmp ; then
    sudo umount ./tmp
fi
mkdir -p ./tmp
sudo mount ${LOOPDEV}p1 ./tmp
cp "$MLO" ./tmp/MLO
sync
sudo umount ./tmp
rmdir ./tmp

sudo losetup -d $LOOPDEV
echo "Done creating $IMG"


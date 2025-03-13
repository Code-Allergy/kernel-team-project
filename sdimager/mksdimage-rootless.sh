#!/usr/bin/env bash
set -euo pipefail

MLO=${1:-}
KERNEL=${2:-}
IMG=${3:-}

# verify all parameters are provided
if [ -z $MLO ] || [ -z $KERNEL ] || [ -z $IMG ]; then
    echo "Usage: $0 <mlo_file> <output_file> <kernel_file>"
    exit 1
fi

if [ ! -f $MLO ]; then
    echo "Error: $MLO not found"
    exit 1
fi

if [ ! -f $KERNEL ]; then
    echo "Error: $KERNEL not found"
    exit 1
fi

# Partition parameters
BOOT_PART_SIZE_MB=50

# For now, unused
ROOT_PART_SIZE_MB=50

IMG_SIZE_MB=$((BOOT_PART_SIZE_MB + ROOT_PART_SIZE_MB))

# Assume the start is at sector 2048
START_SECTOR=2048
OFFSET=$((START_SECTOR * 512))

# Create boot partition image
BOOT_IMG="$IMG.boot.img"

# Create disk images
dd if=/dev/zero of=$IMG bs=1M count=$IMG_SIZE_MB status=none
dd if=/dev/zero of=$BOOT_IMG bs=1M count=$BOOT_PART_SIZE_MB status=none

# Partition disk image
echo "Partitioning $IMG..."
fdisk $IMG <<EOF > /dev/null 2>&1
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

# Format boot partition image
echo "Creating partition image..."
mkfs.vfat -F 32 $BOOT_IMG > /dev/null

# Create mtools configuration file
echo "drive c: file=\"$BOOT_IMG\"" > mtools.conf
export MTOOLSRC="$(pwd)/mtools.conf"

# Copy MLO to boot partition image
echo "Copying MLO to boot partition..."
mcopy -o $MLO c:/MLO

echo "Creating boot directory..."
mmd -i $BOOT_IMG ::/boot

# Copy kernel image to boot partition image
echo "Copying kernel image to boot partition..."
mcopy -o $KERNEL c:/boot/kernel.bin

# DD the partition image into the full image
echo "Writing boot partition to disk image..."
dd if=$BOOT_IMG of=$IMG seek=$START_SECTOR bs=512 conv=notrunc status=none

# Clean up
rm mtools.conf
rm $BOOT_IMG

echo "Done creating $IMG"

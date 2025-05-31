#!/bin/bash

set -e

BUILD_DIR="./build"
MOUNT_DIR="./tmp/mnt"
LOOP_DEVICE="/dev/loop20"
EMPTY_DISK_IMAGE="./misc/empty-disk.img"

# Clean up
mkdir -p "$MOUNT_DIR"
sudo umount "$MOUNT_DIR" &>/dev/null || true
sudo losetup -d "$LOOP_DEVICE" &>/dev/null || true

# Create blank disk image and partition
mkdir -p "$BUILD_DIR"
dd if=/dev/zero of="$EMPTY_DISK_IMAGE" bs=1M count=8
chmod 666 "$EMPTY_DISK_IMAGE"
echo -e "o\nn\np\n1\n2048\n\na\nt\n0b\nw\n" | fdisk "$EMPTY_DISK_IMAGE"
sudo losetup "$LOOP_DEVICE" -P "$EMPTY_DISK_IMAGE"

# Create and mount fat32 filesystem
sudo mkfs.vfat -F32 "$LOOP_DEVICE"p1
sudo fsck -fy "$LOOP_DEVICE"p1
sudo mount -o sync "$LOOP_DEVICE"p1 "$MOUNT_DIR"
sleep 0.5

# Install grub
sudo grub-install --no-floppy --modules="part_msdos multiboot" \
    --target=i386-pc --boot-directory="$MOUNT_DIR/boot/" "$LOOP_DEVICE"

# Unmount and cleanup
sudo umount "$MOUNT_DIR"
sudo losetup -d "$LOOP_DEVICE"

# Compress the disk image
gzip -f -9 "$EMPTY_DISK_IMAGE"
mv "$EMPTY_DISK_IMAGE.gz" "$EMPTY_DISK_IMAGE"

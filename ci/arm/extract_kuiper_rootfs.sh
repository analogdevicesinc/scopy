#!/bin/bash
set -ex

# Extract rootfs from ADI Kuiper Linux ARM64 disk image and create a Docker-ready tarball.
# Usage: sudo ./extract_kuiper_rootfs.sh /path/to/image.img [output_dir]

IMAGE_FILE="$1"
OUTPUT_DIR="${2:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)}"

if [ -z "$IMAGE_FILE" ] || [ ! -f "$IMAGE_FILE" ]; then
	echo "Usage: sudo $0 /path/to/kuiper-arm64.img [output_dir]"
	exit 1
fi

MOUNT_POINT=$(mktemp -d)
ROOTFS_TAR="$OUTPUT_DIR/kuiper-rootfs.tar.gz"

# Kuiper ARM64 partition layout (from fdisk -l):
# Partition 2 (rootfs): Start 4218880, Size 5947392 sectors, ext4
ROOTFS_OFFSET=$((512 * 4218880))
ROOTFS_SIZELIMIT=$((512 * 5947392))

echo "Mounting rootfs from $IMAGE_FILE ..."
mount -v -o loop,offset=$ROOTFS_OFFSET,sizelimit=$ROOTFS_SIZELIMIT "$IMAGE_FILE" "$MOUNT_POINT"

echo "Creating tarball at $ROOTFS_TAR ..."
tar -czf "$ROOTFS_TAR" -C "$MOUNT_POINT" .

echo "Cleaning up ..."
umount "$MOUNT_POINT"
rmdir "$MOUNT_POINT"

echo "Done: $ROOTFS_TAR ($(du -h "$ROOTFS_TAR" | cut -f1))"

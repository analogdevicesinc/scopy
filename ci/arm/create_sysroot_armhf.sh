#!/bin/bash -xe

# ARMHF Sysroot Creation Script for Qt6 Cross-Compilation
# ========================================================
# Creates a sysroot from ADI Kuiper Linux armhf image
# Usage: ./create_sysroot_armhf.sh [function_name ...]
#
# Prerequisites:
#   - Run on host: docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
#   - Docker container must use --privileged (for loop mount)
#
# Functions:
#   install_packages    - Install host tools (qemu, mount utils)
#   extract_sysroot     - Extract rootfs from Kuiper disk image
#   configure_sysroot   - Install dev packages inside sysroot via QEMU chroot
#   fix_relativelinks   - Convert absolute symlinks to relative

SRC_SCRIPT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

STAGING_AREA=$SRC_SCRIPT/staging
SYSROOT=$STAGING_AREA/sysroot
TOOLCHAIN_HOST=arm-linux-gnueabihf

# Kuiper image config — update these when using a new image
KUIPER_IMAGE_PATH=${KUIPER_IMAGE_PATH:-$SRC_SCRIPT/image_ADI-Kuiper-Linux-armhf.img}
ROOTFS_START_SECTOR=4218880
ROOTFS_SIZE_SECTORS=5136384

SYSROOT_RELATIVE_LINKS=https://raw.githubusercontent.com/abhiTronix/rpi_rootfs/master/scripts/sysroot-relativelinks.py

install_packages() {
	sudo apt-get update
	sudo apt-get -y install qemu-user-static qemu-system wget unzip
}

extract_sysroot() {
	echo "#######EXTRACT SYSROOT#######"
	if [ ! -f "$KUIPER_IMAGE_PATH" ]; then
		echo "ERROR: Kuiper image not found at $KUIPER_IMAGE_PATH"
		echo "Set KUIPER_IMAGE_PATH or place the image in $SRC_SCRIPT/"
		exit 1
	fi

	sudo rm -rf ${STAGING_AREA}/kuiper
	sudo mkdir -p ${STAGING_AREA}/kuiper

	sudo mount -v -o loop,offset=$((512*$ROOTFS_START_SECTOR)),sizelimit=$((512*$ROOTFS_SIZE_SECTORS)) \
		${KUIPER_IMAGE_PATH} ${STAGING_AREA}/kuiper

	mkdir -p ${SYSROOT}
	sudo cp -arp ${STAGING_AREA}/kuiper/* ${SYSROOT}
	sudo cp /etc/resolv.conf ${SYSROOT}/etc/resolv.conf

	sudo umount ${STAGING_AREA}/kuiper
	sudo rm -rf ${STAGING_AREA}/kuiper
}

configure_sysroot() {
	echo "#######CONFIGURE SYSROOT VIA QEMU CHROOT#######"

	# Mount virtual filesystems needed by dpkg/apt inside the chroot
	sudo mount -t proc proc ${SYSROOT}/proc
	sudo mount -t sysfs sysfs ${SYSROOT}/sys
	sudo mount --bind /dev ${SYSROOT}/dev
	sudo mount --bind /dev/pts ${SYSROOT}/dev/pts

	sudo cp /etc/resolv.conf ${SYSROOT}/etc/resolv.conf
	sudo cp $SRC_SCRIPT/inside_chroot_armhf_qt6.sh ${SYSROOT}/tmp/inside_chroot_armhf_qt6.sh
	sudo chroot ${SYSROOT} /bin/bash /tmp/inside_chroot_armhf_qt6.sh

	# Unmount virtual filesystems
	sudo umount ${SYSROOT}/dev/pts
	sudo umount ${SYSROOT}/dev
	sudo umount ${SYSROOT}/sys
	sudo umount ${SYSROOT}/proc
}

move_and_extract_sysroot() {
	if [ -f $HOME/sysroot.tar.gz ]; then
		mkdir -p $STAGING_AREA
		sudo tar -xf $HOME/sysroot.tar.gz --directory $STAGING_AREA
		rm $HOME/sysroot.tar.gz
	fi
}

fix_relativelinks() {
	echo "#######FIX RELATIVE LINKS#######"
	pushd ${STAGING_AREA}
	[ -f sysroot-relativelinks.py ] || wget $SYSROOT_RELATIVE_LINKS
	chmod +x sysroot-relativelinks.py
	sudo ./sysroot-relativelinks.py ${SYSROOT}
	popd
}

for arg in "$@"; do
	$arg
done

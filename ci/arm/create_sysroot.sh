#!/bin/bash

# ARM Sysroot Creation Script
# ==========================
# Creates a sysroot from ADI Kuiper Linux image for cross-compilation
# Usage: ./create_sysroot.sh [arm32|arm64] [function_name ...]
#
# A sysroot is a directory structure that mimics the target system's root filesystem
# It contains headers, libraries, and tools needed for cross-compilation
#
# Functions available:
#   - install_packages: Install host tools
#   - download_kuiper: Download ADI Kuiper Linux image
#   - install_qemu: Install QEMU for chroot operations
#   - extract_sysroot: Extract rootfs from disk image
#   - configure_sysroot: Install packages inside sysroot
#   - fix_relativelinks: Convert absolute symlinks to relative

set -ex
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

source $SRC_SCRIPT/arm_build_config.sh $1

install_packages(){
	sudo apt update
	sudo apt -y install git wget unzip python3 python-is-python3 2to3
}

download_kuiper(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}

	if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
		echo "Downloading Linux Kuiper arm64 image"
		# Only download if not already present
		if [ ! -f "image_$IMAGE_NAME.zip" ]; then
			wget \
				--progress=bar:force:noscroll \
				--progress=dot:giga \
				${KUIPER_DOWNLOAD_LINK}
		else
			echo "image_$IMAGE_NAME.zip already downloaded"
		fi
		# Extract if .img doesn't exist
		[ -f image_$IMAGE_NAME.img ] || unzip "image_$IMAGE_NAME.zip"
	elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
		echo "Downloading Linux Kuiper arm32 image"
		if [ ! -f "image_$IMAGE_NAME.zip" ]; then
			wget \
				--progress=bar:force:noscroll \
				--progress=dot:giga \
				--header='User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:122.0) Gecko/20100101 Firefox/122.0' \
				--header='Accept-Language: en-US,en;q=0.5' \
				--header='Accept-Encoding: gzip, deflate, br' \
				--header='Connection: keep-alive' \
				--header='Upgrade-Insecure-Requests: 1' \
				--header='Sec-Fetch-Dest: document' \
				--header='Sec-Fetch-Mode: navigate' \
				--header='Sec-Fetch-Site: none' \
				--header='Sec-Fetch-User: ?1' \
				--header='Pragma: no-cache' \
				--header='Cache-Control: no-cache' \
				${KUIPER_DOWNLOAD_LINK}
		else
			echo "image_$IMAGE_NAME.zip already downloaded"
		fi
		[ -f image_$IMAGE_NAME.img ] || unzip image_$IMAGE_NAME.zip
		# Rename to consistent format if needed
		[ -f $IMAGE_NAME.img ] && mv $IMAGE_NAME.img image_$IMAGE_NAME.img
	fi
	popd
}


# QEMU allows us to run ARM binaries on x86 host
# Required for chroot operations to configure the sysroot
install_qemu(){
	sudo apt update
	sudo apt -y install qemu-system qemu-user-static qemu-user
}

# Extract root filesystem from Kuiper disk image
# Disk images contain multiple partitions (boot, rootfs, etc.)
# We need to extract only the rootfs partition which contains
# the libraries, headers, and tools needed for cross-compilation
#
# Process:
# 1. Mount the rootfs partition using loop device with offset
# 2. Copy entire filesystem to sysroot directory
# 3. Add host's DNS configuration for network access
# 4. Clean up temporary files
extract_sysroot(){
	sudo rm -rf ${STAGING_AREA}/kuiper
	sudo mkdir -p ${STAGING_AREA}/kuiper

	IMAGE_FILE=image_$IMAGE_NAME.img

	# Partition layout information obtained with 'file'
	# The rootfs is the second partition in the image
	# offset = start_sector * 512 (sector size)
	# sizelimit = partition_size * 512

	if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
		# arm64 image partition info:
		# Start sector: 4218880, Size: 3571712 sectors
		sudo mount -v -o loop,offset=$((512*4218880)),sizelimit=$((512*3571712)) ${STAGING_AREA}/${IMAGE_FILE} ${STAGING_AREA}/kuiper
	elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
		# arm32 image partition info:
		# Start sector: 4218880, Size: 19947520 sectors
		sudo mount -v -o loop,offset=$((512*4218880)),sizelimit=$((512*19947520)) ${STAGING_AREA}/${IMAGE_FILE} ${STAGING_AREA}/kuiper
	fi

	mkdir -p ${SYSROOT}
	# -a: archive mode
	# -r: recursive
	# -p: preserve permissions
	sudo cp -arp ${STAGING_AREA}/kuiper/* ${SYSROOT}

	# Copy host's DNS configuration for package downloads in chroot
	sudo cp /etc/resolv.conf ${SYSROOT}/etc/resolv.conf

	sudo umount ${STAGING_AREA}/kuiper
	sudo rm -rf ${STAGING_AREA}/kuiper
	rm -rf ${STAGING_AREA:?}/${IMAGE_FILE}
	rm -rf ${STAGING_AREA}/image*.zip
}

# Uses chroot to execute commands inside the ARM filesystem
# This allows us to use ARM's package manager to install/remove packages
configure_sysroot(){
	if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
			cat $SRC_SCRIPT/inside_chroot_arm64.sh | sudo chroot ${SYSROOT}
	elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
			cat $SRC_SCRIPT/inside_chroot_armhf.sh | sudo chroot ${SYSROOT}
	fi
}


# The tarball is created by Docker image build and cached
# Used in CI to skip the sysroot creation process
move_and_extract_sysroot(){
	if [ -f $HOME/sysroot.tar.gz ]; then
		mkdir -p $STAGING_AREA
		sudo tar -xf $HOME/sysroot.tar.gz --directory $STAGING_AREA
		rm $HOME/sysroot.tar.gz
	fi
}

# Fix absolute symbolic links in sysroot
fix_relativelinks(){
	pushd ${STAGING_AREA}
	[ -f sysroot-relativelinks.py ] || wget $SYSROOT_RELATIVE_LINKS
	chmod +x sysroot-relativelinks.py
	sudo ./sysroot-relativelinks.py ${SYSROOT}
	popd
}

for arg in "${@:2}"; do
	$arg
done


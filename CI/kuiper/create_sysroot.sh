#!/bin/bash

set -ex
SRC_DIR=$(git rev-parse --show-toplevel)
source $SRC_DIR/CI/kuiper/kuiper_build_config.sh

IMAGE_FILE=2023-12-13-ADI-Kuiper-full.img

install_packages(){
	sudo apt update
	sudo apt -y install git wget unzip python3 python
}

download_kuiper(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	wget --progress=dot:giga ${KUIPER_DOWNLOAD_LINK}
	unzip image*.zip
	popd
}

# install qemu needed for the sysroot configuration
install_qemu(){
	sudo apt update
	sudo apt -y install qemu qemu-system qemu-user-static qemu-user
}

# mount the Kuiper image and copy the entire rootfs partition
extract_sysroot(){
	sudo mkdir -p /mnt/kuiper

	# with file ${IMAGE_FILE} we can see the start sector (4218880) and the length (19947520) of the second partition contained in the Kuiper image
	# using this info we can directly mount that partition
	sudo mount -v -o loop,offset=$((512*4218880)),sizelimit=$((512*19947520)) ${STAGING_AREA}/${IMAGE_FILE} /mnt/kuiper

	mkdir -p ${SYSROOT}
	sudo cp -arp /mnt/kuiper/* ${SYSROOT}
	sudo cp /etc/resolv.conf ${SYSROOT}/etc/resolv.conf
	sudo umount /mnt/kuiper
	sudo rm -rf /mnt/kuiper
	rm -rf ${STAGING_AREA:?}/${IMAGE_FILE}
	rm -rf ${STAGING_AREA}/image*.zip
}


# execute chroot inside the sysroot folder and install/remove packages using apt
configure_sysroot(){
	cat $SRC_DIR/CI/kuiper/inside_chroot.sh | sudo chroot ${SYSROOT}
}

move_and_extract_sysroot(){
	if [ -f $HOME/sysroot.tar.gz ]; then
		mkdir -p $STAGING_AREA
		sudo tar -xf $HOME/sysroot.tar.gz --directory $STAGING_AREA
		rm $HOME/sysroot.tar.gz
	fi
}

fix_relativelinks(){
	pushd ${STAGING_AREA}
	wget $SYSROOT_RELATIVE_LINKS
	chmod +x sysroot-relativelinks.py
	sudo ./sysroot-relativelinks.py ${SYSROOT}
	popd
}

for arg in $@; do
	$arg
done


#!/bin/bash

set -ex
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

source $SRC_SCRIPT/arm_build_config.sh $1

IMAGE_FILE=2023-12-13-ADI-Kuiper-full.img

install_packages(){
	sudo apt update
	sudo apt -y install git wget unzip python3 python-is-python3 2to3
}

download_kuiper(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}

	if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
		echo "Waiting for a arm64 Kuiper linux release"
		exit
	elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
		if [ ! -f image_2023-12-13-ADI-Kuiper-full.zip ]; then
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
		fi
	fi

	[ -f $IMAGE_FILE ] || unzip image*.zip
	popd
}

# install qemu needed for the sysroot configuration
install_qemu(){
	sudo apt update
	sudo apt -y install qemu qemu-system qemu-user-static qemu-user
}

# mount the Kuiper image and copy the entire rootfs partition
extract_sysroot(){
	sudo rm -rf ${STAGING_AREA}/kuiper
	sudo mkdir -p ${STAGING_AREA}/kuiper

	# with file ${IMAGE_FILE} we can see the start sector (4218880) and the length (19947520) of the second partition contained in the Kuiper image
	# using this info we can directly mount that partition
	sudo mount -v -o loop,offset=$((512*4218880)),sizelimit=$((512*19947520)) ${STAGING_AREA}/${IMAGE_FILE} ${STAGING_AREA}/kuiper

	mkdir -p ${SYSROOT}
	sudo cp -arp ${STAGING_AREA}/kuiper/* ${SYSROOT}
	sudo cp /etc/resolv.conf ${SYSROOT}/etc/resolv.conf
	sudo umount ${STAGING_AREA}/kuiper
	sudo rm -rf ${STAGING_AREA}/kuiper
	rm -rf ${STAGING_AREA:?}/${IMAGE_FILE}
	rm -rf ${STAGING_AREA}/image*.zip
}

# execute chroot inside the sysroot folder and install/remove packages using apt
configure_sysroot(){
	if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
			cat $SRC_SCRIPT/inside_chroot_arm64.sh | sudo chroot ${SYSROOT}
	elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
			cat $SRC_SCRIPT/inside_chroot_armhf.sh | sudo chroot ${SYSROOT}
	fi
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
	[ -f sysroot-relativelinks.py ] || wget $SYSROOT_RELATIVE_LINKS
	chmod +x sysroot-relativelinks.py
	sudo ./sysroot-relativelinks.py ${SYSROOT}
	popd
}

for arg in "${@:2}"; do
	$arg
done


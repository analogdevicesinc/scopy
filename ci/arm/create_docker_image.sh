#!/bin/bash

set -ex
SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )
ARCH=$1
source $SRC_DIR/ci/arm/arm_build_config.sh "$ARCH"

# install docker
install_packages(){
	sudo apt-get update
	sudo apt-get -y install apt-transport-https ca-certificates curl software-properties-common
	curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
	sudo add-apt-repository --yes "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
	sudo apt-get update
	sudo apt-get -y install containerd.io docker-ce docker-ce-cli docker-buildx-plugin
}

create_sysroot(){
	$SRC_DIR/ci/arm/create_sysroot.sh "$ARCH" \
		install_packages \
		download_kuiper \
		install_qemu \
		extract_sysroot \
		configure_sysroot
}

# archive the sysroot and move it next to the Dockerfile in order to copy the tar in the docker image
tar_and_move_sysroot(){
	pushd $STAGING_AREA
	sudo tar -czf "${SYSROOT_TAR##*/}" sysroot
	sudo mv $SYSROOT_TAR $SRC_DIR/ci/arm
	popd
}

create_image(){
	pushd ${SRC_DIR}/ci/arm
	if [ "${ARCH}" == "arm32" ]; then
		DOCKER_TAG=cristianbindea/scopy2-armhf-appimage:testing
	elif [ "${ARCH}" == "arm64" ]; then
		DOCKER_TAG=cristianbindea/scopy2-arm64-appimage:testing
	fi

	# docker build \
	# 	--load \
	# 	--progress plain \
	# 	--tag ${DOCKER_TAG} \
	# 	-f docker/Dockerfile \
	# 	--build-arg ARCH=${ARCH} .

	# build the image using old backend
	DOCKER_BUILDKIT=0 docker build \
		--tag ${DOCKER_TAG} \
		-f docker/Dockerfile \
		--build-arg ARCH=${ARCH} .
	popd
}

install_packages
create_sysroot
tar_and_move_sysroot
create_image

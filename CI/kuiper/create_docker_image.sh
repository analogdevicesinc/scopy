#!/bin/bash

set -ex
SRC_DIR=$(git rev-parse --show-toplevel)
source $SRC_DIR/CI/kuiper/kuiper_build_config.sh

# install docker
install_packages(){
	sudo apt-get update
	sudo apt-get -y install apt-transport-https ca-certificates curl software-properties-common
	curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
	sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
	sudo apt-get update
	sudo apt-get -y install containerd.io docker-ce docker-ce-cli docker-buildx-plugin
}

create_sysroot(){
	$SRC_DIR/CI/kuiper/create_sysroot.sh \
		install_packages \
		download_kuiper \
		install_qemu \
		extract_sysroot \
		configure_sysroot

}

# archive the sysroot and move it next to Dockerfile in order to copy the tar in the docker image
tar_and_move_sysroot(){
	pushd $STAGING_AREA
	sudo tar -czvf "${SYSROOT_TAR##*/}" sysroot
	sudo mv $SYSROOT_TAR $SYSROOT_DOCKER
	popd
}

create_image(){
	pushd ${SRC_DIR}/CI/kuiper/docker
	sudo docker build --load --tag cristianbindea/scopy1-kuiper .
	# sudo DOCKER_BUILDKIT=0 docker build --tag cristianbindea/scopy1-kuiper . # build the image using old backend
	popd
}

install_packages
create_sysroot
tar_and_move_sysroot
create_image

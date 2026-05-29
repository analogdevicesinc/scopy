#!/bin/bash -ex

# Create Docker Image for Ubuntu Build (Qt6)
# ===========================
# Usage: ./create_docker_image.sh ubuntu22 ubuntu24 ubuntu26

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

ubuntu22(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu22-qt6:testing \
		--build-arg BASE_IMAGE=ubuntu:22.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu24(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6:testing \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu26(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu26-qt6:testing \
		--build-arg BASE_IMAGE=ubuntu:26.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

for arg in $@; do
	$arg
done

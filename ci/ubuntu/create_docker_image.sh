#!/bin/bash -ex

# Create Docker Image for Ubuntu Build (Qt6)
# ===========================
# Usage: ./create_docker_image.sh ubuntu22 ubuntu24 ubuntu26
#        TAG=slim ./create_docker_image.sh ubuntu24
#
# TAG defaults to "testing", the staging tag a freshly built image lands on. It is an environment
# variable rather than $1 because $1 is the target function name consumed by the dispatch loop at
# the bottom - same convention as ci/arm/create_docker_image_qt6.sh. main currently builds against
# :slim, so that tag is still live and must remain buildable.

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

TAG=${TAG:-testing}
IMAGE_BASE=docker.cloudsmith.io/adi/scopy-dockers

ubuntu22(){
	pushd $SRC_SCRIPT
	docker build \
		-t $IMAGE_BASE/scopy2-ubuntu22-qt6:$TAG \
		--build-arg BASE_IMAGE=ubuntu:22.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu24(){
	pushd $SRC_SCRIPT
	docker build \
		-t $IMAGE_BASE/scopy2-ubuntu24-qt6:$TAG \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu26(){
	pushd $SRC_SCRIPT
	docker build \
		-t $IMAGE_BASE/scopy2-ubuntu26-qt6:$TAG \
		--build-arg BASE_IMAGE=ubuntu:26.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu22_qt6(){ ubuntu22; }
ubuntu24_qt6(){ ubuntu24; }
ubuntu26_qt6(){ ubuntu26; }

for arg in $@; do
	$arg
done

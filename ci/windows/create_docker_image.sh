#!/bin/bash -ex

# Create Docker Image for Windows Build (MinGW, Qt6)
# ===========================
# Usage: ./create_docker_image.sh windows
#        TAG=slim ./create_docker_image.sh windows
#
# TAG defaults to "testing", the staging tag a freshly built image lands on; main currently builds
# against :slim, so that tag is still live and must remain buildable. Same convention as
# ci/arm/create_docker_image_qt6.sh and ci/ubuntu/create_docker_image.sh.
#
# Requires Docker Desktop switched to WINDOWS CONTAINERS. The image is a servercore base with
# MSYS2 + MinGW64 inside, so it cannot be built from a Linux daemon. hyperv isolation and a
# generous memory cap are required - the dependency build is memory hungry.

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

TAG=${TAG:-testing}
# Note the image name has no scopy2- prefix, unlike every other image:
# .github/workflows/mingwbuild.yml pulls this exact name.
IMAGE=docker.cloudsmith.io/adi/scopy-dockers/scopy-windows-qt6

windows(){
	pushd $SRC_SCRIPT
	docker build \
		-t $IMAGE:$TAG \
		--isolation=hyperv \
		--memory=16GB \
		-f docker/Dockerfile .
	popd
}

windows_qt6(){ windows; }

for arg in $@; do
	$arg
done

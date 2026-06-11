#!/bin/bash -ex

# Create Docker Image for Windows Build (Qt6)
# ===========================
# Usage: ./create_docker_image.sh windows_qt6

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

windows_qt6(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy-windows-qt6:testing \
		-f docker/Dockerfile .
	popd
}

for arg in $@; do
	$arg
done

#!/bin/bash -ex

# Create Docker Image for the Android (Qt6) APK Build
# ===================================================
# Usage: ./create_docker_image.sh android_qt6
#        TAG=qcoro-test ./create_docker_image.sh android_qt6
#
# TAG defaults to "testing", the staging tag a freshly built image lands on. It is an environment
# variable rather than $1 because $1 is the target function name consumed by the dispatch loop at
# the bottom - same convention as ci/ubuntu/create_docker_image.sh.

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

TAG=${TAG:-testing}
IMAGE_BASE=docker.cloudsmith.io/adi/scopy-dockers

android(){
	# Context is ci/android so the Dockerfile can COPY android_toolchain.sh and
	# android_deps_build.sh. Single ABI (arm64-v8a), set by ENV in the Dockerfile.
	pushd $SRC_SCRIPT
	docker build \
		-t $IMAGE_BASE/scopy2-android-qt6:$TAG \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker/Dockerfile .
	popd
}

android_qt6(){ android; }

for arg in $@; do
	$arg
done

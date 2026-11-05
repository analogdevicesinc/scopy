#!/bin/bash -ex

# Create Docker Image for the Android (Qt6) APK Build
# ===================================================
# Usage: ./create_docker_image.sh android_qt6
#        docker push --all-tags docker.cloudsmith.io/adi/scopy-dockers/scopy2-android-qt6
#
# Tags the image as both :testing (ci_*/scopy_qt6 branches) and :latest (main and others).

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

IMAGE_BASE=docker.cloudsmith.io/adi/scopy-dockers

android(){
	# Context is ci/android so the Dockerfile can COPY android_toolchain.sh and
	# android_deps_build.sh. Single ABI (arm64-v8a), set by ENV in the Dockerfile.
	pushd $SRC_SCRIPT
	docker build \
		-t $IMAGE_BASE/scopy2-android-qt6:testing \
		-t $IMAGE_BASE/scopy2-android-qt6:latest \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker/Dockerfile .
	popd
}

android_qt6(){ android; }

for arg in $@; do
	$arg
done

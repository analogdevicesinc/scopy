#!/bin/bash -ex

# Create Docker Images for ARM Qt6 Builds
# =========================================
# Usage: ./create_docker_image_qt6.sh arm64
#        ./create_docker_image_qt6.sh armhf_cross
#
# The image tag defaults to "testing". The dependency-rework pass uses "slim":
#   TAG=slim ./create_docker_image_qt6.sh arm64
#   ./create_docker_image_qt6.sh arm64_slim
# The tag is an environment variable rather than $1 because $1 is the target function name
# consumed by the dispatch loop at the bottom. The variant lives in the tag, not the image
# name - slim replaces the full image once validated, so a name suffix would have to be
# renamed at promotion and would break every pull URL.

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

TAG=${TAG:-testing}
IMAGE_BASE=docker.cloudsmith.io/adi/scopy-dockers

# Native aarch64 image. Must be built on an arm64 host: Dockerfile.arm64 unpacks a Kuiper
# aarch64 rootfs and compiles the whole dependency stack natively. ci/arm/setup_pi_for_docker_build.sh
# produces both tarballs on a Raspberry Pi 5 and writes them into this directory, which is where
# Dockerfile.arm64 reads them from (the build context root) - so there is nothing to download.
arm64(){
	pushd $SRC_SCRIPT
	for tarball in kuiper-rootfs.tar.gz qt6-arm64-installed.tar.gz; do
		if [ ! -f "$tarball" ]; then
			echo "Missing $SRC_SCRIPT/$tarball"
			echo "Run ./setup_pi_for_docker_build.sh on an arm64 Kuiper Pi to produce it."
			exit 1
		fi
	done

	docker build \
		--progress plain \
		-t $IMAGE_BASE/scopy2-arm64-native-qt6:$TAG \
		-f docker/Dockerfile.arm64 \
		.
	popd
}

armhf_cross(){
	pushd $SRC_SCRIPT
	mkdir -p docker/tarballs

	# Download pre-built tarballs from Cloudsmith
	cloudsmith download adi/scopy-dockers/sysroot-armhf-1.0.0.tar.gz docker/tarballs/sysroot-armhf.tar.gz
	cloudsmith download adi/scopy-dockers/qt6-armhf-host-6.8.3.tar.gz docker/tarballs/qt6-host-installed.tar.gz
	cloudsmith download adi/scopy-dockers/qt6-armhf-cross-6.8.3.tar.gz docker/tarballs/qt6-armhf-cross-installed.tar.gz

	docker build \
		--progress plain \
		-t $IMAGE_BASE/scopy2-armhf-cross-qt6:$TAG \
		-f docker/Dockerfile.armhf-cross \
		.
	popd
}

# Slim dependency images (dependency-rework pass), mirroring ci/ubuntu/create_docker_image.sh.
arm64_slim(){ TAG=slim; arm64; }
armhf_cross_slim(){ TAG=slim; armhf_cross; }

for arg in $@; do
	$arg
done

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

# Cross-compiled ARM32 image. Builds on any x86_64 host: Dockerfile.armhf-cross is FROM
# ubuntu:24.04 with crossbuild-essential-armhf and three prebuilt tarballs, and the armhf qmake6 in
# the Qt prefix is a shell wrapper that execs the host qmake6 - so no qemu and no binfmt.
# The tarballs are produced locally by the four-stage flow in armhf_docker_build_process.md and are
# read straight from docker/tarballs/, so there is nothing to download. The previous
# `cloudsmith download` calls fetched three package slugs whose existence could not be verified,
# and would have overwritten the local inputs. Note that .github/workflows/push-docker.yml keeps
# its own downloads of the same slugs and must keep them: a CI runner has no local copies.
armhf_cross(){
	pushd $SRC_SCRIPT
	for tarball in sysroot-armhf.tar.gz qt6-host-installed.tar.gz qt6-armhf-cross-installed.tar.gz; do
		if [ ! -f "docker/tarballs/$tarball" ]; then
			echo "Missing $SRC_SCRIPT/docker/tarballs/$tarball"
			echo "See ci/arm/armhf_docker_build_process.md - stage 1 produces sysroot-armhf.tar.gz,"
			echo "stage 2 produces the two Qt6 tarballs. They take hours, so they are gitignored"
			echo "rather than merely untracked, to survive a git clean."
			exit 1
		fi
	done

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

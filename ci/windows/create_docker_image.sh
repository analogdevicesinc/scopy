#!/bin/bash -ex

# Create Docker Image for Windows Build (MinGW, Qt6)
# ===========================
# Usage: ./create_docker_image.sh windows_slim
#
# Requires Docker Desktop switched to WINDOWS CONTAINERS. The image is a servercore base with
# MSYS2 + MinGW64 inside, so it cannot be built from a Linux daemon. hyperv isolation and a
# generous memory cap are required - the dependency build is memory hungry.

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Slim dependency image (dependency-rework pass). Same image name as the full variant, with the
# variant in the tag: slim replaces the full image once validated, so a name suffix would have to
# be renamed at promotion and would break every pull URL. Tagged for Cloudsmith so it can be
# pushed directly; this target does not push.
windows_slim(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy-windows-qt6:slim \
		--isolation=hyperv \
		--memory=16GB \
		-f docker/Dockerfile .
	popd
}

windows_qt6_slim(){ windows_slim; }

# Local-only tag kept as a dev escape hatch (same content, no Cloudsmith path).
windows_slim_local(){
	pushd $SRC_SCRIPT
	docker build \
		-t scopy-windows-qt6:deps-rework-local \
		--isolation=hyperv \
		--memory=16GB \
		-f docker/Dockerfile .
	popd
}

for arg in $@; do
	$arg
done

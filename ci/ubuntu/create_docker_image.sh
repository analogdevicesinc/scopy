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

ubuntu22_qt6(){ ubuntu22; }
ubuntu24_qt6(){ ubuntu24; }
ubuntu26_qt6(){ ubuntu26; }

# Slim dependency images (dependency-rework pass). Same image names as the full variant, with the
# variant in the tag: slim replaces the full images once validated, so a name suffix would have to
# be renamed at promotion and would break every pull URL. Tagged for Cloudsmith so they can be
# pushed directly; these targets do not push.
ubuntu22_slim(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu22-qt6:slim \
		--build-arg BASE_IMAGE=ubuntu:22.04 \
		-f docker_ubuntu/Dockerfile.slim .
	popd
}

ubuntu24_slim(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6:slim \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker_ubuntu/Dockerfile.slim .
	popd
}

ubuntu26_slim(){
	pushd $SRC_SCRIPT
	docker build \
		-t docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu26-qt6:slim \
		--build-arg BASE_IMAGE=ubuntu:26.04 \
		-f docker_ubuntu/Dockerfile.slim .
	popd
}

ubuntu22_qt6_slim(){ ubuntu22_slim; }
ubuntu24_qt6_slim(){ ubuntu24_slim; }
ubuntu26_qt6_slim(){ ubuntu26_slim; }

all_ubuntu_slim(){
	ubuntu22_slim
	ubuntu24_slim
	ubuntu26_slim
}

# Local-only tag kept as a dev escape hatch (unchanged behaviour, no Cloudsmith path).
ubuntu24_slim_local(){
	pushd $SRC_SCRIPT
	docker build \
		-t scopy2-ubuntu24-qt6:deps-rework-local \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker_ubuntu/Dockerfile.slim .
	popd
}

for arg in $@; do
	$arg
done

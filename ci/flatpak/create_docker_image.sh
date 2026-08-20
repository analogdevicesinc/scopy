#!/bin/bash -xe

# Create Docker Image for Flatpak Build
# ===========================
# Usage: ./create_docker_image.sh [tag]
#
# The tag defaults to "testing". The dependency-rework pass uses "slim":
#   ./create_docker_image.sh slim
# The variant lives in the tag, not the image name - slim replaces the full image once
# validated, so a name suffix would have to be renamed at promotion and would break
# every pull URL.

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

TAG=${1:-testing}
IMAGE=docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6:$TAG

build_docker(){
	pushd $SRC_DIR/ci/flatpak/docker
	docker build -t $IMAGE  .
	CONTAINER_NAME=builder-flatpak-$TAG
	# A failed dependency build aborts this script under -e, so the "docker container rm"
	# below never runs and the exited container blocks the next "docker run --name".
	# Remove it here instead of at the end, so a failed container survives for inspection
	# until the next attempt.
	docker rm -f $CONTAINER_NAME 2>/dev/null || true
	docker run \
		--name $CONTAINER_NAME \
		--privileged \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/build_flatpak_deps.sh",target=/home/runner/config/build_flatpak_deps.sh,readonly \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/defined_variables.h",target=/home/runner/config/defined_variables.h,readonly \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/org.adi.Scopy.json.c",target=/home/runner/config/org.adi.Scopy.json.c,readonly \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/Makefile",target=/home/runner/config/Makefile,readonly \
		$IMAGE /bin/bash -c /home/runner/config/build_flatpak_deps.sh

	docker commit $CONTAINER_NAME $IMAGE
	echo "Docker container $CONTAINER_NAME commited as $IMAGE image"
	docker container rm $CONTAINER_NAME
	popd
}

build_docker
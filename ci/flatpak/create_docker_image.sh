#!/bin/bash -xe

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

build_docker(){
	pushd $SRC_DIR/ci/flatpak/docker
	docker build -t cristianbindea/scopy2-flatpak:testing  .
	CONTAINER_NAME=builder-flatpak
	docker run \
		--name $CONTAINER_NAME \
		--privileged \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/build_flatpak_deps.sh",target=/home/runner/config/build_flatpak_deps.sh,readonly \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/defined_variables.h",target=/home/runner/config/defined_variables.h,readonly \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/org.adi.Scopy.json.c",target=/home/runner/config/org.adi.Scopy.json.c,readonly \
		--mount type=bind,source="$SRC_DIR/ci/flatpak/Makefile",target=/home/runner/config/Makefile,readonly \
		cristianbindea/scopy2-flatpak:testing /bin/bash -c /home/runner/config/build_flatpak_deps.sh

	docker commit $CONTAINER_NAME cristianbindea/scopy2-flatpak:testing
	echo "Docker container $CONTAINER_NAME commited as cristianbindea/scopy2-flatpak:testing image"
	docker container rm $CONTAINER_NAME
	popd
}

build_docker
#!/bin/bash -ex

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

ubuntu20(){
	pushd $SRC_SCRIPT
	docker build \
		-t cristianbindea/scopy2-x86_64-appimage-ubuntu20:testing \
		--build-arg BASE_IMAGE=ubuntu:20.04 \
		-f docker/Dockerfile .
	popd
}

ubuntu24(){
	pushd $SRC_SCRIPT
	docker build \
		-t cristianbindea/scopy2-x86_64-appimage:testing \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker/Dockerfile .
	popd
}

for arg in $@; do
	$arg
done

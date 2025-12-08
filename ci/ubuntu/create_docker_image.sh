#!/bin/bash -ex

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

ubuntu20(){
	pushd $SRC_SCRIPT
	docker build \
		-t cristianbindea/scopy2-ubuntu20:testing \
		--build-arg BASE_IMAGE=ubuntu:20.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu22(){
	pushd $SRC_SCRIPT
	docker build \
		-t cristianbindea/scopy2-ubuntu22:testing \
		--build-arg BASE_IMAGE=ubuntu:22.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

ubuntu24(){
	pushd $SRC_SCRIPT
	docker build \
		-t cristianbindea/scopy2-ubuntu24:testing \
		--build-arg BASE_IMAGE=ubuntu:24.04 \
		-f docker_ubuntu/Dockerfile .
	popd
}

for arg in $@; do
	$arg
done

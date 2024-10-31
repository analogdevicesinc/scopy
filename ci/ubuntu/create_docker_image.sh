#!/bin/bash -ex

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

ubuntu20(){
	pushd $SRC_SCRIPT
	docker build -t cristianbindea/scopy2-ubuntu20:testing -f docker_ubuntu20/Dockerfile .
	popd
}

ubuntu22(){
	pushd $SRC_SCRIPT
	docker build -t cristianbindea/scopy2-ubuntu22:testing -f docker_ubuntu22/Dockerfile .
	popd
}

for arg in $@; do
	$arg
done

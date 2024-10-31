#!/bin/bash -ex

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
pushd $SRC_SCRIPT

docker build -t cristianbindea/scopy2-x86_64-appimage:testing -f docker/Dockerfile .

popd

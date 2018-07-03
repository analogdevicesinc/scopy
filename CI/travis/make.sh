#!/bin/bash
set -e

NUM_JOBS=4
STAGINGDIR="${PWD}/deps/staging"

mkdir -p build

pushd build

cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
	-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" ..

CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS}

popd

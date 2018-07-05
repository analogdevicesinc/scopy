#!/bin/bash

# if we have a Qt59 installation use it
if [ -f /opt/qt59/bin/qt59-env.sh ] ; then
	source /opt/qt59/bin/qt59-env.sh
fi

set -e

source ./CI/travis/lib.sh

NUM_JOBS=4

mkdir -p build

pushd build

cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
	-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" ..

CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS}

popd

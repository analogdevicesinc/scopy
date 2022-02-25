#!/bin/bash

JOBS=$JOBS
STAGINGDIR="/home/appveyor/projects/scopy/deps/staging_dir"

if [ $# -eq 0 ]; then
	echo "Using default qmake"
	QMAKE=qmake
	CMAKE_PREFIX_PATH_PARAM=
	$QMAKE --version
else
	QMAKE=$1/gcc_64/bin/qmake
	CMAKE_PREFIX_PATH_PARAM=-DCMAKE_PREFIX_PATH="$1/gcc_64/lib/cmake;$STAGINGDIR/lib/pkgconfig;$STAGINGDIR/lib/cmake;$STAGINGDIR/lib/x86_64-linux-gnu/pkgconfig;$STAGINGDIR"
	$QMAKE --version

fi

cd ~/projects/scopy
mkdir build
cd build
cmake $CMAKE_PREFIX_PATH_PARAM -DCMAKE_STAGING_PREFIX="${STAGINGDIR}" -DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" ../
make $JOBS


#!/bin/bash

JOBS=$JOBS

if [ $# -eq 0 ]; then
	echo "Using default qmake"
	QMAKE=qmake
	CMAKE_PREFIX_PATH_PARAM=
	$QMAKE --version
else
	QMAKE=$1/gcc_64/bin/qmake
	CMAKE_PREFIX_PATH_PARAM=-DCMAKE_PREFIX_PATH\=$1/gcc_64/lib/cmake
	$QMAKE --version

fi

cd ~/projects/scopy
mkdir build
cd build
cmake $CMAKE_PREFIX_PATH_PARAM ../
make $JOBS


#!/bin/bash
set -e

# Use the /usr/local/bin/pkg-config instead of /Mono/../pkg-config
export PATH="/usr/local/bin:$PATH"

# if we have a Qt59 installation use it
if [ -f /opt/qt59/bin/qt59-env.sh ] ; then
	. /opt/qt59/bin/qt59-env.sh
fi

if command -v brew ; then
	QT_PATH="$(brew --prefix qt)/bin"
	export PATH="${QT_PATH}:$PATH"
fi

NUM_JOBS=4

mkdir -p build

cd build

if [ "$APPVEYOR" == "true" ] ; then
	cmake ..
	make -j${NUM_JOBS}
else
	cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR;${QT_PATH}/lib/cmake" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" ..
	CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS}
fi


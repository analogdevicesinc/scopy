#!/bin/bash
set -ex
REPO_SRC=$(git rev-parse --show-toplevel)
source $REPO_SRC/ci/macOS/macos_config.sh

build_iio-emu(){
	echo "### Clone and Build IIO-Emulator"
	pushd $REPO_SRC
	if [ ! -d "$REPO_SRC/iio-emu" ]; then
		git clone https://github.com/analogdevicesinc/iio-emu $REPO_SRC/iio-emu
	fi
	mkdir -p $REPO_SRC/iio-emu/build
	cd $REPO_SRC/iio-emu/build
	cmake \
		-DCMAKE_LIBRARY_PATH="$STAGING_AREA_DEPS" \
		-DCMAKE_INSTALL_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_PREFIX_PATH="${STAGING_AREA_DEPS};${STAGING_AREA_DEPS}/lib/cmake;" \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_STAGING_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGING_AREA_DEPS}/lib" \
		../
	CFLAGS=-I${STAGING_AREA_DEPS}/include LDFLAGS=-L${STAGING_AREA_DEPS}/lib make ${JOBS}
	popd
}

build_scopy(){
	echo "### Building Scopy"
	ls -la $REPO_SRC
	pushd $REPO_SRC

	rm -rf $REPO_SRC/build
	mkdir -p $REPO_SRC/build
	cd $REPO_SRC/build
	cmake \
		-DCMAKE_LIBRARY_PATH="$STAGING_AREA_DEPS" \
		-DCMAKE_INSTALL_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_PREFIX_PATH="${STAGING_AREA_DEPS};${STAGING_AREA_DEPS}/lib/cmake;${STAGING_AREA_DEPS}/lib/pkgconfig;${STAGING_AREA_DEPS}/lib/cmake/gnuradio;${STAGING_AREA_DEPS}/lib/cmake/iio" \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_STAGING_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGING_AREA_DEPS}/lib" \
		-DENABLE_TESTING=OFF \
		../
	CFLAGS=-I${STAGING_AREA_DEPS}/include LDFLAGS=-L${STAGING_AREA_DEPS}/lib make ${JOBS}
	otool -l ./Scopy.app/Contents/MacOS/Scopy
	otool -L ./Scopy.app/Contents/MacOS/Scopy
	popd
}

build_iio-emu
build_scopy

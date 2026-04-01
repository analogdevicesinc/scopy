#!/bin/bash

# macOS Build Script
# =====================================
# Build Scopy and IIO-Emulator on macOS
# Usage: ./build_azure_macos.sh
#
# This script is designed for Azure Pipelines CI but can be run locally on macOS systems.
# It assumes dependencies are already installed via install_macos_deps.sh

set -ex
REPO_SRC=$(git rev-parse --show-toplevel)
# Load macOS-specific configuration
source $REPO_SRC/ci/macOS/macos_config.sh

# Build IIO Emulator
# =================
# Virtual IIO device for testing without hardware
build_iio-emu(){
	echo "### Clone and Build IIO-Emulator"
	pushd $REPO_SRC
	# Clone if not present
	if [ ! -d "$REPO_SRC/iio-emu" ]; then
		git clone https://github.com/analogdevicesinc/iio-emu $REPO_SRC/iio-emu
	fi
	mkdir -p $REPO_SRC/iio-emu/build
	cd $REPO_SRC/iio-emu/build

	# Configure with CMake
	# macOS needs explicit library and include paths
	cmake \
		-DCMAKE_LIBRARY_PATH="$STAGING_AREA_DEPS" \
		-DCMAKE_INSTALL_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_PREFIX_PATH="${STAGING_AREA_DEPS};${STAGING_AREA_DEPS}/lib/cmake;" \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_STAGING_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGING_AREA_DEPS}/lib" \
		-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
		../

	# Build with explicit paths for macOS
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
		-DCMAKE_INSTALL_PREFIX="$STAGING_AREA/scopy-install" \
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

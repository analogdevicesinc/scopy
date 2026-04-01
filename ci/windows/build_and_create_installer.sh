#!/bin/bash

# Windows Installer Creation Script
# ================================
# Build Scopy and create Windows installer package
# Usage: ./build_and_create_installer.sh [function_name ...]
#
# This script:
# 1. Builds Scopy and IIO-Emulator
# 2. Collects all dependencies (DLLs, Qt plugins, etc.)
# 3. Bundles device drivers
# 4. Creates Inno Setup installer
# 5. Generates debug symbols package

if [ "$CI_SCRIPT" == "ON" ];
	then
		set -ex
		export WORKDIR=$HOME
		SRC_FOLDER=$WORKDIR/scopy
	else
		set -x
		SRC_FOLDER=$(git rev-parse --show-toplevel)
		export WORKDIR=$SRC_FOLDER
fi

BUILD_TARGET=x86_64
ARCH_BIT=64

## Staging configuration
USE_STAGING=OFF
##

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/mingw_toolchain.sh $USE_STAGING

INSTALL_FOLDER=$STAGING_AREA/scopy-install      # Where 'make install' puts files
BUILD_FOLDER=$WORKDIR/build_$ARCH               # CMake build directory
ARTIFACT_FOLDER=$SRC_FOLDER/artifacts           # Final output location
export DEST_FOLDER=$ARTIFACT_FOLDER/scopy-$ARCH # Bundle directory (exported for Inno Setup)
DEBUG_FOLDER=$ARTIFACT_FOLDER/debug-$ARCH       # Unstripped binaries for debugging
PYTHON_FILES=$STAGING_DIR/lib/python3.*         # Python runtime
EMU_BUILD_FOLDER=$WORKDIR/iio-emu/build         # IIO emulator build directory

download_tools() {
	mkdir -p $STAGING_AREA
	pacman -Qs mingw-w64-x86_64-wget2 > /dev/null || pacman -S --noconfirm mingw-w64-x86_64-wget2

	pushd $STAGING_AREA
	if [ ! -f windres.exe ]; then
		wget2 http://swdownloads.analog.com/cse/build/windres.exe.gz
		gunzip windres.exe.gz
	fi

	if [ ! -f dpinst.zip ]; then
		wget2 http://swdownloads.analog.com/cse/m1k/drivers/dpinst.zip
		unzip "dpinst.zip"
	fi

	if [ ! -f dfu-util.zip ]; then
		wget2 http://swdownloads.analog.com/cse/m1k/drivers/dfu-util.zip
		unzip "dfu-util.zip"
	fi

	if [ ! -f cv2pdb-dlls.zip ]; then
		wget2 https://swdownloads.analog.com/cse/scopydeps/cv2pdb-dlls.zip
		unzip "cv2pdb-dlls.zip"
	fi
	popd
}


# Configures and builds the main Scopy executable
build_scopy(){
	echo "### Building Scopy"
	download_cmake
	# Copy build status from dependencies build
	[ -f $HOME/build-status ] && cp $HOME/build-status $SRC_FOLDER/build-status
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER

	$CMAKE $RC_COMPILER_OPT -DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe \
				-DENABLE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER \
				$SRC_FOLDER
	$MAKE_BIN $JOBS
	ls -la $BUILD_FOLDER
}

build_iio-emu(){
	echo "### Building IIO-EMU"
	download_cmake
	# Clone if not present
	if [ ! -d "$WORKDIR/iio-emu" ]; then
		git clone https://github.com/analogdevicesinc/iio-emu $WORKDIR/iio-emu
	fi

	mkdir -p $EMU_BUILD_FOLDER
	cd $EMU_BUILD_FOLDER
	$CMAKE -DBUILD_TOOLS=ON ../
	$MAKE_BIN $JOBS

	pushd $WORKDIR/iio-emu
	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd
}

# Bundle device drivers with installer
bundle_drivers(){
	echo "### Bundling drivers"
	cp -R $SRC_FOLDER/ci/windows/drivers $DEST_FOLDER
	if [[ $ARCH_BIT == "64" ]]; then
		cp -R $STAGING_AREA/dfu-util-static-amd64.exe $DEST_FOLDER/drivers/dfu-util.exe
		cp -R $STAGING_AREA/dpinst_amd64.exe $DEST_FOLDER/drivers/dpinst.exe
	else
		cp -R $STAGING_AREA/dfu-util-static.exe $DEST_FOLDER/drivers/dfu-util.exe
		cp -R $STAGING_AREA/dpinst.exe $DEST_FOLDER/drivers/dpinst.exe
	fi
}


# Assembles all files needed for the Windows distribution
deploy_app(){
	echo "### Deploying application and dependencies"

	rm -rf $INSTALL_FOLDER
	pushd $BUILD_FOLDER
	make $JOBS install
	popd

	rm -rf $DEST_FOLDER
	mkdir -p $DEST_FOLDER

	cp -v $INSTALL_FOLDER/bin/Scopy.exe $DEST_FOLDER/          # GUI version
	cp -v $INSTALL_FOLDER/bin/Scopy-console.exe $DEST_FOLDER/  # GUI with console output (for debugging)
	cp -v $INSTALL_FOLDER/bin/qt.conf $DEST_FOLDER/            # Qt configuration
	cp -v $EMU_BUILD_FOLDER/iio-emu.exe $DEST_FOLDER/          # IIO emulator
	cp -v $EMU_BUILD_FOLDER/tools/iio-emu_gen_xml.exe $DEST_FOLDER/  # XML generator

	# This Qt tool automatically finds and copies required Qt DLLs and plugins
	$STAGING_DIR/bin/windeployqt.exe \
		--dir $DEST_FOLDER \
		--no-translations \
		--no-system-d3d-compiler \
		--no-compiler-runtime \
		--no-quick-import \
		--opengl \
		--printsupport \
		$DEST_FOLDER/Scopy.exe

	cp -vr $INSTALL_FOLDER/lib/libscopy*.dll $DEST_FOLDER
	cp -vr $INSTALL_FOLDER/lib/scopy/* $DEST_FOLDER
	cp -vr $INSTALL_FOLDER/resources $DEST_FOLDER              # Resources (filters, etc.)
	cp -vr $STAGING_DIR/share/libsigrokdecode/decoders  $DEST_FOLDER/
	rm -vfr $(find $DEST_FOLDER -name "*.dll.a" -type f)

	# Copy additional Qt plugins not handled by windeployqt
	cp -vr  $STAGING_DIR/share/qt5/plugins/renderers $DEST_FOLDER/
	cp -vr  $STAGING_DIR/share/qt5/plugins/sceneparsers $DEST_FOLDER/

	# List is maintained in mingw_dll_deps file
	pushd $STAGING_DIR/bin
	source $SRC_FOLDER/ci/windows/mingw_dll_deps
	cp -vn "${DLL_DEPS[@]}" $DEST_FOLDER/
	cp -nv iio_*.exe $DEST_FOLDER/
	popd

	if [ "$USE_STAGING" == "ON" ]; then
		cp -v $STAGING_DIR/bin/libgenalyzer.dll $DEST_FOLDER/ || echo "Warning: genalyzer DLL not found in staging"
	else
		cp -v /$MINGW_VERSION/bin/libgenalyzer.dll $DEST_FOLDER/ || echo "Warning: genalyzer DLL not found in system"
	fi

	cp -r $PYTHON_FILES $DEST_FOLDER
	cp $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss $DEST_FOLDER
	cp $SRC_FOLDER/LICENSE $DEST_FOLDER/LICENSE.txt

	bundle_drivers
}


# This reduces installer size while preserving debugging capability
extract_debug_symbols(){
	echo "### Duplicating unstripped bundle"
	rm -rf $DEBUG_FOLDER
	mkdir -p $DEBUG_FOLDER
	cp -r $DEST_FOLDER/* $DEBUG_FOLDER/

	echo "### Stripping bundle for installer"
	# --strip-debug: Remove debugging symbols
	# --strip-unneeded: Remove all symbols not needed for relocation
	/$MINGW_VERSION/bin/strip.exe --verbose --strip-debug --strip-unneeded $(find $DEST_FOLDER -name "*.exe" -type f)
	/$MINGW_VERSION/bin/strip.exe --verbose --strip-debug --strip-unneeded $(find $DEST_FOLDER -name "*.dll" -type f)
}

# Create Windows installer using Inno Setup
create_installer() {
	echo "### Creating installer"
	pushd $ARTIFACT_FOLDER
	# Add Inno Setup to PATH
	PATH="/c/innosetup:/c/Program Files (x86)/Inno Setup 6:$PATH"
	# Compile installer script
	# //p flag shows progress
	iscc //p $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss

	if [ "$CI_SCRIPT" == "ON" ]; then
		mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER
		zip scopy-$ARCH_BIT-setup.zip scopy-$ARCH_BIT-setup.exe
		pushd $DEBUG_FOLDER
		zip -r $ARTIFACT_FOLDER/debug-x86_64.zip .
		popd
		pushd $DEST_FOLDER
		zip -r $ARTIFACT_FOLDER/scopy-x86_64.zip .
		popd
	else
		mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER
	fi

	ls -la $ARTIFACT_FOLDER
	echo "Done. Artifacts generated in $ARTIFACT_FOLDER"
	popd
}

# In Docker Image, tools are pre-downloaded in the Docker image
move_tools(){
	[ -d /home/docker/staging ] && mv /home/docker/staging $STAGING_AREA || echo "Staging folder not found or already moved"
	if [ ! -d $STAGING_AREA ]; then
		echo "Missing tools folder, downloading now"
		download_tools
	fi
}

run_workflow(){
	# Setup tools (move or download)
	[ "$CI_SCRIPT" == "ON" ] && move_tools || download_tools
	# Build steps
	build_scopy          # Build main application
	build_iio-emu        # Build emulator
	deploy_app           # Collect all files
	extract_debug_symbols # Strip binaries
	create_installer     # Create installer
}

for arg in $@; do
	$arg
done

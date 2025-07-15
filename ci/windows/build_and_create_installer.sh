#!/bin/bash
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

## Set STAGING
USE_STAGING=OFF
##

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/mingw_toolchain.sh $USE_STAGING

INSTALL_FOLDER=$STAGING_AREA/scopy-install
BUILD_FOLDER=$WORKDIR/build_$ARCH
ARTIFACT_FOLDER=$SRC_FOLDER/artifacts
export DEST_FOLDER=$ARTIFACT_FOLDER/scopy-$ARCH # the export is needed for the packaging step
DEBUG_FOLDER=$ARTIFACT_FOLDER/debug-$ARCH
PYTHON_FILES=$STAGING_DIR/lib/python3.*
EMU_BUILD_FOLDER=$WORKDIR/iio-emu/build

download_tools() {
	mkdir -p $STAGING_AREA

	# check if wget2 is installed
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

build_scopy(){
	echo "### Building Scopy"
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

deploy_app(){
	echo "### Deploying application and dependencies"

	rm -rf $INSTALL_FOLDER
	pushd $BUILD_FOLDER
	make $JOBS install
	popd

	rm -rf $DEST_FOLDER
	mkdir -p $DEST_FOLDER
	cp -v $INSTALL_FOLDER/bin/Scopy.exe $DEST_FOLDER/
	cp -v $INSTALL_FOLDER/bin/Scopy-console.exe $DEST_FOLDER/
	cp -v $INSTALL_FOLDER/bin/qt.conf $DEST_FOLDER/
	cp -v $EMU_BUILD_FOLDER/iio-emu.exe $DEST_FOLDER/
	cp -v $EMU_BUILD_FOLDER/tools/iio-emu_gen_xml.exe $DEST_FOLDER/

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
	cp -vr $INSTALL_FOLDER/resources $DEST_FOLDER
	cp -vr $STAGING_DIR/share/libsigrokdecode/decoders  $DEST_FOLDER/

	pushd $STAGING_DIR/bin
	DLL_DEPS=$(cat $SRC_FOLDER/ci/windows/mingw_dll_deps)
	cp -vn $DLL_DEPS $DEST_FOLDER/
	cp -nv iio_*.exe $DEST_FOLDER/
	popd

	cp -r $PYTHON_FILES $DEST_FOLDER
	cp $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss $DEST_FOLDER

	bundle_drivers
}

extract_debug_symbols(){
	echo "### Duplicating unstripped bundle"
	rm -rf $DEBUG_FOLDER
	mkdir -p $DEBUG_FOLDER
	cp -r $DEST_FOLDER/* $DEBUG_FOLDER/
	echo "### Stripping bundle for installer"
	/$MINGW_VERSION/bin/strip.exe --verbose --strip-debug --strip-unneeded $(find $DEST_FOLDER -name "*.exe" -type f)
	/$MINGW_VERSION/bin/strip.exe --verbose --strip-debug --strip-unneeded $(find $DEST_FOLDER -name "*.dll" -type f)
}

create_installer() {
	echo "### Creating installer"
	pushd $ARTIFACT_FOLDER
	PATH="/c/innosetup:/c/Program Files (x86)/Inno Setup 6:$PATH"
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

# move the staging folder that contains the tools needed for the build to the known location
move_tools(){
	[ -d /home/docker/staging ] && mv /home/docker/staging $STAGING_AREA || echo "Staging folder not found or already moved"
	if [ ! -d $STAGING_AREA ]; then
		echo "Missing tools folder, downloading now"
		download_tools
	fi
}


run_workflow(){
	[ "$CI_SCRIPT" == "ON" ] && move_tools || download_tools
	build_scopy
	build_iio-emu
	deploy_app
	extract_debug_symbols
	create_installer
}

for arg in $@; do
	$arg
done

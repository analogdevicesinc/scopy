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
		git clone https://github.com/analogdevicesinc/scopy-mingw-build-deps.git -b ci-for-scopy2 $WORKDIR/scopy-mingw-build-deps
fi

BUILD_TARGET=x86_64
ARCH_BIT=64

## Set STAGING
USE_STAGING=OFF
##

TOOLS_FOLDER=$WORKDIR/scopy-mingw-build-deps
pushd $TOOLS_FOLDER
source ./mingw_toolchain.sh $BUILD_TARGET $USE_STAGING
popd

export DEST_FOLDER=$WORKDIR/scopy_$ARCH
BUILD_FOLDER=$WORKDIR/build_$ARCH
DEBUG_FOLDER=$WORKDIR/debug_$ARCH
ARTIFACT_FOLDER=$WORKDIR/artifact_$ARCH
PYTHON_FILES=$STAGING_DIR/lib/python3.*
DLL_DEPS=$(cat $SRC_FOLDER/ci/windows/mingw_dll_deps)
EMU_BUILD_FOLDER=$WORKDIR/iio-emu/build
STAGING_AREA=$SRC_FOLDER/ci/windows/staging
REGMAP_XMLS=$BUILD_FOLDER/plugins/plugins/regmap/xmls

# Generate build status info for the about page
cp $BUILD_STATUS_FILE $SRC_FOLDER/build-status
pacman -Qe >> $SRC_FOLDER/build-status

build_scopy(){
	echo "### Building Scopy"
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $RC_COMPILER_OPT -DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe \
				-DENABLE_TESTING=OFF \
				$SRC_FOLDER
	$MAKE_BIN -j4
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
	$MAKE_BIN -j4
}

download_tools() {
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	if [ ! -f windres.exe ]; then
		wget http://swdownloads.analog.com/cse/build/windres.exe.gz
		gunzip windres.exe.gz
	fi

	if [ ! -f dpinst.zip ]; then
		wget http://swdownloads.analog.com/cse/m1k/drivers/dpinst.zip
		unzip "dpinst.zip"
	fi

	if [ ! -f dfu-util.zip ]; then
		wget http://swdownloads.analog.com/cse/m1k/drivers/dfu-util.zip
		unzip "dfu-util.zip"
	fi

	if [ ! -f cv2pdb-dlls.zip ]; then
		wget https://swdownloads.analog.com/cse/scopydeps/cv2pdb-dlls.zip
		unzip "cv2pdb-dlls.zip"
	fi

	if [ ! -f is.exe ]; then
		wget https://jrsoftware.org/download.php/is.exe
	fi
	popd
}

deploy_app(){
	echo "### Deploying application and dependencies"
	if [ -d $DEST_FOLDER ]; then
		rm -rf $DEST_FOLDER
	fi
	rm -rf $DEST_FOLDER
	mkdir -p $DEST_FOLDER
	cp $BUILD_FOLDER/Scopy.exe $DEST_FOLDER/
	cp $BUILD_FOLDER/qt.conf $DEST_FOLDER/
	cp $BUILD_FOLDER/Scopy-console.exe $DEST_FOLDER/

	mkdir $DEST_FOLDER/resources
	$STAGING_DIR/bin/windeployqt.exe --dir $DEST_FOLDER --no-translations --no-system-d3d-compiler --no-compiler-runtime --no-quick-import --opengl --printsupport $BUILD_FOLDER/Scopy.exe

	cp -r $STAGING_DIR/share/libsigrokdecode/decoders  $DEST_FOLDER/

	pushd $STAGING_DIR/bin
	cp -n $DLL_DEPS $DEST_FOLDER/
	cp -n iio_*.exe $DEST_FOLDER/
	popd

	cp $EMU_BUILD_FOLDER/iio-emu.exe $DEST_FOLDER
	cp -r $PYTHON_FILES $DEST_FOLDER
	cp $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss $DEST_FOLDER
	cp -v $BUILD_FOLDER/libscopy-*.dll $DEST_FOLDER

	PLUGINS_DLL=$(find $BUILD_FOLDER/plugins/plugins -type f -name "*.dll")
	mkdir -p $DEST_FOLDER/plugins
	cp -v $PLUGINS_DLL $DEST_FOLDER/plugins

	TRANSLATIONS_QM=$(find $BUILD_FOLDER/translations -type f -name "*.qm")
	mkdir -p $DEST_FOLDER/translations
	cp $TRANSLATIONS_QM $DEST_FOLDER/translations

	if [ -d $REGMAP_XMLS ]; then
		cp -r $REGMAP_XMLS $DEST_FOLDER/plugins
	fi
}

extract_debug_symbols(){
	echo "### Duplicating unstripped bundle"
	rm -rf $DEBUG_FOLDER
	mkdir -p $DEBUG_FOLDER
	cp -r $DEST_FOLDER/* $DEBUG_FOLDER/
	echo "### Stripping bundle for installer"
	/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded $DEST_FOLDER/*.exe
	/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded $DEST_FOLDER/*.dll
	/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded $DEST_FOLDER/plugins/*.dll
}

bundle_drivers(){
	echo "### Bundling drivers"
	cp -R $SRC_FOLDER/windows/drivers $DEST_FOLDER
	if [[ $ARCH_BIT == "64" ]]; then
		cp -R $STAGING_AREA/dfu-util-static-amd64.exe $DEST_FOLDER/drivers/dfu-util.exe
		cp -R $STAGING_AREA/dpinst_amd64.exe $DEST_FOLDER/drivers/dpinst.exe
	else
		cp -R $STAGING_AREA/dfu-util-static.exe $DEST_FOLDER/drivers/dfu-util.exe
		cp -R $STAGING_AREA/dpinst.exe $DEST_FOLDER/drivers/dpinst.exe
	fi
}

create_installer() {
	echo "### Creating installer"
	pushd $WORKDIR
	mkdir -p $ARTIFACT_FOLDER
	cp -R $WORKDIR/scopy_${ARCH} $ARTIFACT_FOLDER/scopy-${ARCH}
	[ -d $WORKDIR/debug_${ARCH} ] && cp -R $WORKDIR/debug_${ARCH} $ARTIFACT_FOLDER/debug-${ARCH} || echo "No debug folder"
	PATH="/c/innosetup:/c/Program Files (x86)/Inno Setup 6:$PATH"
	iscc //p $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss
	mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER

	echo "Done. Artifacts generated in $ARTIFACT_FOLDER"
	ls -la $ARTIFACT_FOLDER

	if [ "$CI_SCRIPT" == "ON" ]; then
		cp -R $ARTIFACT_FOLDER $SRC_FOLDER
		ls -la $SRC_FOLDER
	fi
	popd
}

run_workflow(){
	download_tools
	build_scopy
	build_iio-emu
	deploy_app
	bundle_drivers
	extract_debug_symbols
	create_installer
}

run_workflow

for arg in $@; do
	$arg
done
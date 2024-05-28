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
TOOLS_FOLDER=$WORKDIR/scopy-mingw-build-deps
pushd $TOOLS_FOLDER
source ./mingw_toolchain.sh $BUILD_TARGET OFF  # USING_STAGING = OFF
popd

export DEST_FOLDER=$WORKDIR/scopy_$ARCH
BUILD_FOLDER=$WORKDIR/build_$ARCH
DEBUG_FOLDER=$WORKDIR/debug_$ARCH
ARTIFACT_FOLDER=$WORKDIR/artifact_$ARCH
PYTHON_FILES=$STAGING_DIR/lib/python3.*
DLL_DEPS=$(cat $SRC_FOLDER/ci/windows/mingw_dll_deps)
EMU_BUILD_FOLDER=$WORKDIR/iio-emu/build

PLUGINBASE_DLL=$BUILD_FOLDER/pluginbase
CORE_DLL=$BUILD_FOLDER/core
GUI_DLL=$BUILD_FOLDER/gui
M2K_GUI_DLL=$BUILD_FOLDER/plugins/m2k/m2k-gui
SIGROK_GUI_DLL=$BUILD_FOLDER/plugins/m2k/m2k-gui
GR_GUI_DLL=$BUILD_FOLDER/plugins/m2k/m2k-gui
IIOUTIL_DLL=$BUILD_FOLDER/iioutil
COMMON_DLL=$BUILD_FOLDER/common
IIO_WIDGETS_DLL=$BUILD_FOLDER/iio-widgets
GRUTIL_DLL=$BUILD_FOLDER/gr-util

REGMAP_XMLS=$BUILD_FOLDER/plugins/regmap/xmls

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

deploy_app(){
	echo "### Deploying application and dependencies"
	if [ -d $DEST_FOLDER ]; then
		rm -rf $DEST_FOLDER
	fi
	mkdir $DEST_FOLDER
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
	cp $PLUGINBASE_DLL/libscopy-pluginbase.dll $DEST_FOLDER
	cp $CORE_DLL/libscopy-core.dll $DEST_FOLDER
	cp $GUI_DLL/libscopy-gui.dll $DEST_FOLDER
	cp $IIOUTIL_DLL/libscopy-iioutil.dll $DEST_FOLDER
	cp $M2K_GUI_DLL/libscopy-m2k-gui.dll $DEST_FOLDER
	cp $SIGROK_GUI_DLL/libscopy-sigrok-gui.dll $DEST_FOLDER
	cp $GR_GUI_DLL/libscopy-gr-gui.dll $DEST_FOLDER
	cp $COMMON_DLL/libscopy-common.dll $DEST_FOLDER
	cp $GRUTIL_DLL/libscopy-gr-util.dll $DEST_FOLDER
	cp $IIO_WIDGETS_DLL/libscopy-iio-widgets.dll $DEST_FOLDER

	PLUGINS_DLL=$(find $BUILD_FOLDER/plugins -type f -name "*.dll")
	mkdir -p $DEST_FOLDER/plugins/plugins
	cp $PLUGINS_DLL $DEST_FOLDER/plugins/plugins

	TRANSLATIONS_QM=$(find $BUILD_FOLDER/translations -type f -name "*.qm")
	mkdir -p $DEST_FOLDER/translations
	cp $TRANSLATIONS_QM $DEST_FOLDER/translations

	if [ -d $REGMAP_XMLS ]; then
		cp -r $REGMAP_XMLS $DEST_FOLDER/plugins/plugins
	fi
}

extract_debug_symbols(){
	echo "### Duplicating unstripped bundle"
	mkdir -p $DEBUG_FOLDER
	cp -r $DEST_FOLDER/* $DEBUG_FOLDER/
	echo "### Stripping bundle for installer"
	/c/msys64/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded $DEST_FOLDER/*.exe
	/c/msys64/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded $DEST_FOLDER/*.dll
	/c/msys64/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded $DEST_FOLDER/plugins/plugins/*.dll
}

bundle_drivers(){
	echo "### Bundling drivers"
	cp -R $SRC_FOLDER/windows/drivers $DEST_FOLDER
	if [[ $ARCH_BIT == "64" ]]; then
		cp -R $TOOLS_FOLDER/dfu-util-static-amd64.exe $DEST_FOLDER/drivers/dfu-util.exe
		cp -R $TOOLS_FOLDER/dpinst_amd64.exe $DEST_FOLDER/drivers/dpinst.exe
	else
		cp -R $TOOLS_FOLDER/dfu-util-static.exe $DEST_FOLDER/drivers/dfu-util.exe
		cp -R $TOOLS_FOLDER/dpinst.exe $DEST_FOLDER/drivers/dpinst.exe
	fi
}

create_installer() {
	echo "### Creating installer"
	mkdir -p $ARTIFACT_FOLDER
	cd $WORKDIR
	cp -R $WORKDIR/scopy_${ARCH} $ARTIFACT_FOLDER/scopy-${ARCH}
	cp -R $WORKDIR/debug_${ARCH} $ARTIFACT_FOLDER/debug-${ARCH}
	PATH="/c/innosetup:/c/Program Files (x86)/Inno Setup 6:$PATH"
	iscc //p $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss
	mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER

	echo "Done. Artifacts generated in $ARTIFACT_FOLDER"
	ls -la $ARTIFACT_FOLDER
	cp -R $ARTIFACT_FOLDER $SRC_FOLDER
	ls -la $SRC_FOLDER
}

build_scopy
build_iio-emu
deploy_app
bundle_drivers
extract_debug_symbols
create_installer

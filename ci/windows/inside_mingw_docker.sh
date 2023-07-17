#!/bin/bash

set -ex

TOOLS_FOLDER=$HOME/scopy-mingw-build-deps
pushd $TOOLS_FOLDER
source ./mingw_toolchain.sh $BUILD_TARGET OFF  # USING_STAGING = OFF
popd

WORKDIR=$HOME
SRC_FOLDER=$WORKDIR/scopy
DEST_FOLDER=$WORKDIR/scopy_$ARCH
BUILD_FOLDER=$WORKDIR/build_$ARCH
DEBUG_FOLDER=$WORKDIR/debug_$ARCH
ARTIFACT_FOLDER=$WORKDIR/artifact_$ARCH
PYTHON_FILES=$STAGING_DIR/lib/python3.*
DLL_DEPS=$(cat $SRC_FOLDER/ci/windows/mingw_dll_deps)

PLUGINBASE_DLL=$BUILD_FOLDER/pluginbase
CORE_DLL=$BUILD_FOLDER/core
GUI_DLL=$BUILD_FOLDER/gui
SIGROK_GUI_DLL=$BUILD_FOLDER/gui/sigrok-gui
GR_GUI_DLL=$BUILD_FOLDER/gui/gr-gui
IIOUTIL_DLL=$BUILD_FOLDER/iioutil
COMMON_DLL=$BUILD_FOLDER/common

REGMAP_XMLS=$BUILD_FOLDER/plugins/regmap/xmls

# Generate build status info for the about page
cp $BUILD_STATUS_FILE $SRC_FOLDER/build-status
pacman -Qe >> $SRC_FOLDER/build-status

build_scopy(){
	echo "### Building Scopy"
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $RC_COMPILER_OPT -DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe $SRC_FOLDER
	$MAKE_BIN -j 4
	ls -la $BUILD_FOLDER
}

deploy_app(){
	echo "### Deploying application and dependencies"
	if [ -d $DEST_FOLDER ]; then
		rm -rf $DEST_FOLDER
	fi
	mkdir $DEST_FOLDER
	cp $BUILD_FOLDER/Scopy.exe $DEST_FOLDER/
	cp $BUILD_FOLDER/qt.conf $DEST_FOLDER/

	mkdir $DEST_FOLDER/resources
	$STAGING_DIR/bin/windeployqt.exe --dir $DEST_FOLDER --no-system-d3d-compiler --no-compiler-runtime --no-quick-import --opengl --printsupport $BUILD_FOLDER/Scopy.exe

	cp -r $STAGING_DIR/share/libsigrokdecode/decoders  $DEST_FOLDER/

	pushd $STAGING_DIR/bin
	cp -n $DLL_DEPS $DEST_FOLDER/
	popd

	cp -r $PYTHON_FILES $DEST_FOLDER
	cp $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss $DEST_FOLDER
	cp $PLUGINBASE_DLL/libscopy-pluginbase.dll $DEST_FOLDER
	cp $CORE_DLL/libscopy-core.dll $DEST_FOLDER
	cp $GUI_DLL/libscopy-gui.dll $DEST_FOLDER
	cp $IIOUTIL_DLL/libscopy-iioutil.dll $DEST_FOLDER
	cp $SIGROK_GUI_DLL/libscopy-sigrok-gui.dll $DEST_FOLDER
	cp $GR_GUI_DLL/libscopy-gr-gui.dll $DEST_FOLDER
	cp $COMMON_DLL/libscopy-common.dll $DEST_FOLDER

	PLUGINS_DLL=$(find $BUILD_FOLDER/plugins -type f -name "*.dll")
	mkdir -p $DEST_FOLDER/plugins/plugins
	cp $PLUGINS_DLL $DEST_FOLDER/plugins/plugins

	if [ -d $REGMAP_XMLS ]; then
		cp -r $REGMAP_XMLS $DEST_FOLDER/plugins/plugins
	fi
}

extract_debug_symbols(){
	echo "### Extracting debug symbols"
	mkdir -p $DEST_FOLDER/.debug
	$STAGING_DIR/bin/dump_syms -r $DEST_FOLDER/Scopy.exe > $DEST_FOLDER/Scopy.exe.sym
	#/$MINGW_VERSION/bin/objcopy -v --only-keep-debug /c/$DEST_FOLDER/Scopy.exe /c/$DEST_FOLDER/.debug/Scopy.exe.debug
	#/c/msys64/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded /c/$DEST_FOLDER/Scopy.exe
	#/c/msys64/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded /c/$DEST_FOLDER/*.dll
	#/c/msys64/$MINGW_VERSION/bin/objcopy.exe -v --add-gnu-debuglink=/c/$DEST_FOLDER/.debug/Scopy.exe.debug /c/$DEST_FOLDER/Scopy.exe
	mkdir $DEBUG_FOLDER
	mv $DEST_FOLDER/Scopy.exe.sym $DEBUG_FOLDER
	mv $DEST_FOLDER/.debug $DEBUG_FOLDER
	$TOOLS_FOLDER/cv2pdb/cv2pdb $DEST_FOLDER/Scopy.exe
	$TOOLS_FOLDER/cv2pdb/cv2pdb $DEST_FOLDER/libm2k.dll
	$TOOLS_FOLDER/cv2pdb/cv2pdb $DEST_FOLDER/libiio.dll
	$TOOLS_FOLDER/cv2pdb/cv2pdb $DEST_FOLDER/libgnuradio-m2k.dll
	$TOOLS_FOLDER/cv2pdb/cv2pdb $DEST_FOLDER/libgnuradio-scopy.dll
	$TOOLS_FOLDER/cv2pdb/cv2pdb $PLUGINS_DLL
	cp -R $DEST_FOLDER/scopy $DEBUG_FOLDER/scopy
	mv $DEST_FOLDER/*.pdb $DEBUG_FOLDER

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
#	cp -R $WORKDIR/debug_${ARCH} $ARTIFACT_FOLDER/debug-${ARCH}
	PATH=/c/innosetup:$PATH
	iscc //p $BUILD_FOLDER/windows/scopy-$ARCH_BIT.iss
	mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER

	echo "Done. Artifacts generated in $ARTIFACT_FOLDER"
	ls -la $ARTIFACT_FOLDER
	echo $GITHUB_WORKSPACE
	cp -R $ARTIFACT_FOLDER $SRC_FOLDER
	ls -la $SRC_FOLDER
}

build_scopy
deploy_app
bundle_drivers
create_installer
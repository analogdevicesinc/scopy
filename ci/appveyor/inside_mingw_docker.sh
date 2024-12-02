#!/bin/bash

set -ex


TOOLS_FOLDER=$HOME/scopy-mingw-build-deps
pushd $TOOLS_FOLDER
source ./mingw_toolchain.sh $BUILD_TARGET ON  # USING_STAGING = ON
popd

WORKDIR=$HOME
SRC_FOLDER=$WORKDIR/scopy
DEST_FOLDER=$WORKDIR/scopy_$ARCH
BUILD_FOLDER=$WORKDIR/build_$ARCH
DEBUG_FOLDER=$WORKDIR/debug_$ARCH
ARTIFACT_FOLDER=$WORKDIR/artifact_$ARCH
PYTHON_FILES=$STAGING_DIR/lib/python3.*
DLL_DEPS=$(cat $SRC_FOLDER/CI/appveyor/mingw_dll_deps)

# Generate build status info for the about page
cp $BUILD_STATUS_FILE $SRC_FOLDER/build-status
pacman -Qe >> $SRC_FOLDER/build-status

echo "### Building Scopy "
mkdir -p $BUILD_FOLDER
cd $BUILD_FOLDER
$CMAKE $RC_COMPILER_OPT -DBREAKPAD_HANDLER=ON -DWITH_DOC=ON -DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe $SRC_FOLDER
$MAKE_BIN  -j 4


echo "### Deploying application and dependencies"
mkdir $DEST_FOLDER
cp $BUILD_FOLDER/Scopy.exe $DEST_FOLDER/
cp $BUILD_FOLDER/qt.conf $DEST_FOLDER/
mkdir $DEST_FOLDER/resources

cp $BUILD_FOLDER/iio-emu/iio-emu.exe $DEST_FOLDER/

# windeployqt was broken in qt version 5.14.2 - it should be fixed in Qt 5.15 - https://bugreports.qt.io/browse/QTBUG-80763
$STAGING_DIR/bin/windeployqt.exe --dir $DEST_FOLDER --no-system-d3d-compiler --no-compiler-runtime --no-quick-import --opengl --printsupport $BUILD_FOLDER/Scopy.exe
cp -r $STAGING_DIR/share/libsigrokdecode/decoders  $DEST_FOLDER/

#tar -C /c/$DEST_FOLDER --strip-components=3 -xJf /c/scopy-$MINGW_VERSION-build-deps.tar.xz msys64/$MINGW_VERSION/bin
cd $STAGING_DIR/bin
cp -r -n $DLL_DEPS $DEST_FOLDER/
cp -r $PYTHON_FILES $DEST_FOLDER
cp $BUILD_FOLDER/scopy-$ARCH_BIT.iss $DEST_FOLDER

echo "### Extracting debug symbols ..."
mkdir -p $DEST_FOLDER/.debug
#/$MINGW_VERSION/bin/objcopy -v --only-keep-debug /c/$DEST_FOLDER/Scopy.exe /c/$DEST_FOLDER/.debug/Scopy.exe.debug
$STAGING_DIR/bin/dump_syms -r $DEST_FOLDER/Scopy.exe > $DEST_FOLDER/Scopy.exe.sym
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
cp -R $DEST_FOLDER/scopy $DEBUG_FOLDER/scopy
mv $DEST_FOLDER/*.pdb $DEBUG_FOLDER

echo "### Bundling drivers"
cp -R $SRC_FOLDER/drivers $DEST_FOLDER
if [[ $ARCH_BIT == "64" ]]; then
        cp -R $TOOLS_FOLDER/dfu-util-static-amd64.exe $DEST_FOLDER/drivers/dfu-util.exe
        cp -R $TOOLS_FOLDER/dpinst_amd64.exe $DEST_FOLDER/drivers/dpinst.exe
else
        cp -R $TOOLS_FOLDER/dfu-util-static.exe $DEST_FOLDER/drivers/dfu-util.exe
        cp -R $TOOLS_FOLDER/dpinst.exe $DEST_FOLDER/drivers/dpinst.exe
fi


echo "### Creating installer... "
mkdir -p $ARTIFACT_FOLDER
cd $WORKDIR
cp -R $WORKDIR/scopy_${ARCH} $ARTIFACT_FOLDER/scopy-${ARCH}
cp -R $WORKDIR/debug_${ARCH} $ARTIFACT_FOLDER/debug-${ARCH}
PATH=/c/innosetup:$PATH
iscc //p $BUILD_FOLDER/scopy-$ARCH_BIT.iss
mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER

echo "Done. Artifacts generated in $ARTIFACT_FOLDER"
ls -la $ARTIFACT_FOLDER
echo $GITHUB_WORKSPACE
cp -R $ARTIFACT_FOLDER $SRC_FOLDER
ls -la $SRC_FOLDER

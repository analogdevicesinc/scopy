#!/bin/bash

set -x


TOOLS_FOLDER=$HOME/scopy-mingw-build-deps
pushd $TOOLS_FOLDER
source ./mingw_toolchain.sh $BUILD_TARGET
popd

echo "### Cloning Qt-Advanced-Docking-System "
pushd $WORKDIR
git clone https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/ qadvanceddocking
cd qadvanceddocking
git checkout tags/3.8.1

echo "### Building Qt-Advanced-Docking-System "
CURRENT_BUILD=qadvanceddocking
INSTALL="install"
if [ ! -z $NO_INSTALL ]; then
	INSTALL=""
fi
pushd $WORKDIR/$CURRENT_BUILD

git clean -xdf
rm -rf ${WORKDIR}/$CURRENT_BUILD/build-${ARCH}
mkdir ${WORKDIR}/$CURRENT_BUILD/build-${ARCH}
cd ${WORKDIR}/$CURRENT_BUILD/build-${ARCH}

eval $CURRENT_BUILD_POST_CLEAN
eval $CURRENT_BUILD_PATCHES
$CMAKE $CURRENT_BUILD_CMAKE_OPTS $WORKDIR/$CURRENT_BUILD
eval $CURRENT_BUILD_POST_CMAKE
$MAKE_BIN $JOBS $INSTALL
eval $CURRENT_BUILD_POST_MAKE
echo "$CURRENT_BUILD - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE

popd
rm -rf ${WORKDIR}/$CURRENT_BUILD/build-${ARCH}

popd


WORKDIR=$HOME
SRC_FOLDER=$WORKDIR/scopy
DEST_FOLDER=$WORKDIR/scopy_$ARCH
BUILD_FOLDER=$WORKDIR/build_$ARCH
DEBUG_FOLDER=$WORKDIR/debug_$ARCH
ARTIFACT_FOLDER=$WORKDIR/artifact_$ARCH
PYTHON_FILES=/$MINGW_VERSION/lib/python3.*
DLL_DEPS=$(cat $SRC_FOLDER/CI/appveyor/mingw_dll_deps)



echo "### Building Scopy "
mkdir -p $BUILD_FOLDER
cd $BUILD_FOLDER
$CMAKE $RC_COMPILER_OPT -DBREAKPAD_HANDLER=ON -DWITH_DOC=ON -DPYTHON_EXECUTABLE=/$MINGW_VERSION/bin/python3.exe $SRC_FOLDER
$MAKE_BIN  -j 4


echo "### Deploying application and dependencies"
mkdir $DEST_FOLDER
cp $BUILD_FOLDER/Scopy.exe $DEST_FOLDER/
cp $BUILD_FOLDER/qt.conf $DEST_FOLDER/
mkdir $DEST_FOLDER/resources

cp $BUILD_FOLDER/iio-emu/iio-emu.exe $DEST_FOLDER/

# windeployqt was broken in qt version 5.14.2 - it should be fixed in Qt 5.15 - https://bugreports.qt.io/browse/QTBUG-80763
$STAGING/$MINGW_VERSION/bin/windeployqt.exe --dir $DEST_FOLDER --no-system-d3d-compiler --no-compiler-runtime --no-quick-import --opengl --printsupport $BUILD_FOLDER/Scopy.exe
cp -r $STAGING/$MINGW_VERSION/share/libsigrokdecode/decoders  $DEST_FOLDER/

#tar -C /c/$DEST_FOLDER --strip-components=3 -xJf /c/scopy-$MINGW_VERSION-build-deps.tar.xz msys64/$MINGW_VERSION/bin
cd /$MINGW_VERSION/bin 
cp -r -n $DLL_DEPS $DEST_FOLDER/
cd $STAGING/$MINGW_VERSION/bin 
cp -r $DLL_DEPS $DEST_FOLDER/
cp -r $PYTHON_FILES $DEST_FOLDER


echo "### Extracting debug symbols ..."
mkdir -p $DEST_FOLDER/.debug
#/$MINGW_VERSION/bin/objcopy -v --only-keep-debug /c/$DEST_FOLDER/Scopy.exe /c/$DEST_FOLDER/.debug/Scopy.exe.debug
$STAGING/$MINGW_VERSION/bin/dump_syms -r $DEST_FOLDER/Scopy.exe > $DEST_FOLDER/Scopy.exe.sym
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


echo "### Creating archives & installer... "
mkdir -p $ARTIFACT_FOLDER
cd $WORKDIR
zip -r "$ARTIFACT_FOLDER/scopy-${ARCH}.zip" scopy_$ARCH
zip -r "$ARTIFACT_FOLDER/debug-${ARCH}.zip" debug_$ARCH
PATH=/c/innosetup:$PATH
iscc //p $BUILD_FOLDER/scopy-$ARCH_BIT.iss
mv $WORKDIR/scopy-$ARCH_BIT-setup.exe $ARTIFACT_FOLDER

echo "Done. Artifacts generated in $ARTIFACT_FOLDER"
ls -la $ARTIFACT_FOLDER
echo $GITHUB_WORKSPACE
cp -R $ARTIFACT_FOLDER $SRC_FOLDER
ls -la $SRC_FOLDER

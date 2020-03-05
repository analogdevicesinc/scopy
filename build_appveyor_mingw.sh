#!/usr/bin/bash.exe

set -e

SCOPY_MINGW_BUILD_DEPS_FORK=analogdevicesinc
SCOPY_MINGW_BUILD_DEPS_BRANCH=upgrade-gr38

export PATH=/bin:/usr/bin:/${MINGW_VERSION}/bin:/c/Program\ Files/Git/cmd:/c/Windows/System32:/c/Program\ Files/7-Zip:/c/Program\ Files\ \(x86\)/Inno\ Setup\ \5:/c/Program\ Files/Appveyor/BuildAgent
echo $PATH

WORKDIR=${PWD}
echo BUILD_NO $BUILD_NO
JOBS=-j3

CC=/${MINGW_VERSION}/bin/${ARCH}-w64-mingw32-gcc.exe
CXX=/${MINGW_VERSION}/bin/${ARCH}-w64-mingw32-g++.exe
CMAKE_OPTS="
	-DCMAKE_C_COMPILER:FILEPATH=${CC} \
	-DCMAKE_CXX_COMPILER:FILEPATH=${CXX} \
	-DPKG_CONFIG_EXECUTABLE=/$MINGW_VERSION/bin/pkg-config.exe \
	-DCMAKE_PREFIX_PATH=/c/msys64/$MINGW_VERSION/lib/cmake \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	"

SCOPY_CMAKE_OPTS="
	$RC_COMPILER_OPT \
	-DBREAKPAD_HANDLER=ON \
	-DGIT_EXECUTABLE=/c/Program\\ Files/Git/cmd/git.exe \
	-DPYTHON_EXECUTABLE=/$MINGW_VERSION/bin/python3.exe \
	"

echo "Download and install pre-compiled libraries ... "
wget -q "https://ci.appveyor.com/api/projects/$SCOPY_MINGW_BUILD_DEPS_FORK/scopy-mingw-build-deps/artifacts/scopy-$MINGW_VERSION-build-deps-pacman.txt?branch=$SCOPY_MINGW_BUILD_DEPS_BRANCH&job=Environment: MINGW_VERSION=$MINGW_VERSION, ARCH=$ARCH" -O /c/scopy-$MINGW_VERSION-build-deps-pacman.txt
wget -q "https://ci.appveyor.com/api/projects/$SCOPY_MINGW_BUILD_DEPS_FORK/scopy-mingw-build-deps/artifacts/scopy-$MINGW_VERSION-build-deps.tar.xz?branch=$SCOPY_MINGW_BUILD_DEPS_BRANCH&job=Environment: MINGW_VERSION=$MINGW_VERSION, ARCH=$ARCH" -O /c/scopy-$MINGW_VERSION-build-deps.tar.xz
cd /c
tar xJf scopy-$MINGW_VERSION-build-deps.tar.xz

cd /c
SCOPY_MINGW_BUILD_DEPS_PACMAN=$(<scopy-$MINGW_VERSION-build-deps-pacman.txt)
PACMAN_SYNC_DEPS="
	$SCOPY_MINGW_BUILD_DEPS_PACMAN\
	mingw-w64-$ARCH-matio \
"

PACMAN_REPO_DEPS="
	http://repo.msys2.org/mingw/$ARCH/mingw-w64-$ARCH-breakpad-git-r1680.70914b2d-1-any.pkg.tar.xz \
	http://repo.msys2.org/mingw/$ARCH/mingw-w64-$ARCH-qt5-5.13.2-1-any.pkg.tar.xz \
	http://repo.msys2.org/mingw/$ARCH/mingw-w64-$ARCH-libusb-1.0.21-2-any.pkg.tar.xz \
"

DLL_DEPS="libmatio-*.dll libhdf5-*.dll libszip*.dll libpcre*.dll libdouble-conversion*.dll libwinpthread-*.dll libgcc_*.dll libstdc++-*.dll libboost_*.dll libglib-*.dll libintl-*.dll libiconv-*.dll libglibmm-2.*.dll libgmodule-2.*.dll libgobject-2.*.dll libffi-*.dll libsigc-2.*.dll libfftw3f-*.dll libicu*.dll zlib*.dll libharfbuzz-*.dll libfreetype-*.dll libbz2-*.dll libpng16-*.dll libgraphite2.dll libjpeg-*.dll libsqlite3-*.dll libwebp-*.dll libxml2-*.dll liblzma-*.dll libxslt-*.dll libzip*.dll libpython3.*.dll libgnutls*.dll libnettle*.dll libhogweed*.dll libgmp*.dll libidn*.dll libp11*.dll libtasn*.dll libunistring*.dll libusb-*.dll libzstd*.dll libgnuradio-*.dll /$MINGW_VERSION/lib/python3.* libiio*.dll libvolk*.dll liblog4cpp*.dll libad9361*.dll liborc*.dll libsigrok*.dll qwt*.dll"

#do we need this ?
#http://swdownloads.analog.com/cse/build/windres.exe.gz;\

#if [ ${ARCH} == "i686" ]
#then
#	RC_COMPILER_OPT="-DCMAKE_RC_COMPILER=/c/windres.exe"
#else
#	RC_COMPILER_OPT=""
#fi

OLD_PATH=$PATH
DEST_FOLDER=scopy_$ARCH_BIT
BUILD_FOLDER=build_$ARCH_BIT
DEBUG_FOLDER=debug_$ARCH_BIT

PATH=/c/msys64/$MINGW_VERSION/bin:$PATH
echo "### Installing the dependencies"
pacman --noconfirm -Sy $PACMAN_SYNC_DEPS
pacman --noconfirm -U  $PACMAN_REPO_DEPS

# Download a 32-bit version of windres.exe
cd /c
wget http://swdownloads.analog.com/cse/build/windres.exe.gz
gunzip windres.exe.gz

# Hack: Qt5Qml CMake script throws errors when loading its plugins. So let's just drop those plugins.
rm -f /$MINGW_VERSION/lib/cmake/Qt5Qml/*Factory.cmake

echo "### Building Scopy ..."
/$MINGW_VERSION/bin/python3.exe --version
mkdir /c/$BUILD_FOLDER
cd /c/$BUILD_FOLDER
cmake -G"Unix Makefiles" "$SCOPY_CMAKE_OPTS" $CMAKE_OPTS /c/projects/scopy

cd /c/$BUILD_FOLDER/resources
sed -i  's/^\(FILEVERSION .*\)$/\1,'$BUILD_NO'/' properties.rc
cat properties.rc
cd /c/build_$ARCH_BIT && make $JOBS


echo "### Deploy the application (copy the dependencies) ..."
mkdir /c/$DEST_FOLDER
cp /c/$BUILD_FOLDER/Scopy.exe /c/$DEST_FOLDER/
cp /c/$BUILD_FOLDER/qt.conf /c/$DEST_FOLDER/

/c/msys64/$MINGW_VERSION/bin/windeployqt.exe --dir /c/$DEST_FOLDER --release --no-system-d3d-compiler --no-compiler-runtime --no-quick-import --opengl --printsupport /c/$BUILD_FOLDER/Scopy.exe
cp -r /c/projects/scopy/resources/decoders  /c/$DEST_FOLDER/

#tar -C /c/$DEST_FOLDER --strip-components=3 -xJf /c/scopy-$MINGW_VERSION-build-deps.tar.xz msys64/$MINGW_VERSION/bin
cd /$MINGW_VERSION/bin ;
cp -r $DLL_DEPS /c/$DEST_FOLDER/

echo "### Extracting debug symbols ..."
mkdir /c/scopy_$ARCH_BIT/.debug
#/$MINGW_VERSION/bin/objcopy -v --only-keep-debug /c/$DEST_FOLDER/Scopy.exe /c/$DEST_FOLDER/.debug/Scopy.exe.debug
dump_syms -r /c/$DEST_FOLDER/Scopy.exe > /c/$DEST_FOLDER/Scopy.exe.sym
#/c/msys64/$MINGW_VERSION/bin/strip.exe --strip-debug --strip-unneeded /c/$DEST_FOLDER/Scopy.exe
#/c/msys64/$MINGW_VERSION/bin/objcopy.exe -v --add-gnu-debuglink=/c/$DEST_FOLDER/.debug/Scopy.exe.debug /c/$DEST_FOLDER/Scopy.exe
mkdir /c/$DEBUG_FOLDER
mv /c/$DEST_FOLDER/Scopy.exe.sym /c/$DEBUG_FOLDER
mv /c/$DEST_FOLDER/.debug /c/$DEBUG_FOLDER

echo "### Creating archives ... "
7z a "/c/scopy-${ARCH_BIT}bit.zip" /c/$DEST_FOLDER
# appveyor PushArtifact /c/scopy-${ARCH_BIT}bit.zip
7z a "/c/debug-${ARCH_BIT}bit.zip" /c/$DEBUG_FOLDER
# appveyor PushArtifact /c/debug-${ARCH_BIT}bit.zip
iscc //Qp /c/$BUILD_FOLDER/scopy-$ARCH_BIT.iss
# appveyor PushArtifact /c/$BUILD_FOLDER/scopy-$ARCH_BIT.iss

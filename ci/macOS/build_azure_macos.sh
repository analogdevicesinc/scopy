#!/bin/bash
set -ex

STAGING_AREA=$PWD/staging
STAGING_AREA_DEPS=$STAGING_AREA/dependencies
JOBS=-j4
REPO_SRC=$BUILD_REPOSITORY_LOCALPATH

QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
export PATH="${QT_PATH}:$PATH"

MACOS_VERSION=$(/usr/libexec/PlistBuddy -c "Print:ProductVersion" /System/Library/CoreServices/SystemVersion.plist)
if [[ "$MACOS_VERSION" == "10.14."* ]] ; then
	export MACOSX_DEPLOYMENT_TARGET=10.13
fi

echo "### Building scopy"
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
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH;$STAGING_AREA_DEPS;$STAGING_AREA_DEPS/lib"
CFLAGS=-I${STAGING_AREA_DEPS}/include LDFLAGS=-L${STAGING_AREA_DEPS}/lib make ${JOBS}

otool -l ./Scopy.app/Contents/MacOS/Scopy

popd

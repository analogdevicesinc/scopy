#!/usr/bin/bash.exe

set -ex
# get the full directory path of the script
export WORKFOLDER=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BUILD_STATUS_FILE=$HOME/scopy-mingw-build-status

LIBSERIALPORT_BRANCH=scopy-v2
LIBIIO_VERSION=libiio-v0
LIBAD9361_BRANCH=main
LIBM2K_BRANCH=main
SPDLOG_BRANCH=v1.x
LIBSNDFILE_BRANCH=1.2.2
VOLK_BRANCH=main
GNURADIO_BRANCH=maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=main
LIBSIGROKDECODE_BRANCH=master
QWT_BRANCH=qwt-multiaxes-updated
LIBTINYIIOD_BRANCH=master
IIOEMU_BRANCH=master

STAGING_AREA=$WORKFOLDER/staging
MINGW_VERSION=mingw64
ARCH=x86_64

USE_STAGING=$1
if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		echo -- USING STAGING FOLDER: $STAGING_AREA_DEPS
		export USE_STAGING="ON"
		export STAGING_AREA_DEPS=$STAGING_AREA/dependencies
		export STAGING_DIR=${WORKFOLDER}/${STAGING_AREA_DEPS}/${MINGW_VERSION}
		export PACMAN="pacman -r $STAGING_DIR --noconfirm --needed"
	else
		echo -- NO STAGING: INSTALLING IN SYSTEM
		export USE_STAGING="OFF"
		export STAGING_DIR=/${MINGW_VERSION}
		export PACMAN="pacman --noconfirm --needed"
fi

RC_COMPILER_OPT="-DCMAKE_RC_COMPILER=/mingw64/bin/windres.exe"
PATH="/bin:$STAGING_DIR/bin:$WORKFOLDER/cv2pdb:/c/Program Files (x86)/Inno Setup 6:/c/innosetup/:/bin:/usr/bin:${STAGING_DIR}/bin:/c/Program\ Files/Git/cmd:/c/Windows/System32:/c/Program\ Files/Appveyor/BuildAgent:$PATH"
QMAKE=${STAGING_DIR}/bin/qmake
PKG_CONFIG_PATH=$STAGING_DIR/lib/pkgconfig
CC=${STAGING_DIR}/bin/${ARCH}-w64-mingw32-gcc.exe
CXX=${STAGING_DIR}/bin/${ARCH}-w64-mingw32-g++.exe
JOBS="-j22"
MAKE_BIN=/usr/bin/make.exe
MAKE_CMD="$MAKE_BIN $JOBS"
export CMAKE_GENERATOR="Unix Makefiles"

export CMAKE_OPTS=( \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_VERBOSE_MAKEFILE=ON \
	-DCMAKE_C_COMPILER:FILEPATH=${CC} \
	-DCMAKE_CXX_COMPILER:FILEPATH=${CXX} \
	-DCMAKE_MAKE_PROGRAM:FILEPATH=${MAKE_BIN}\
	-DPKG_CONFIG_EXECUTABLE=${STAGING_DIR}/bin/pkg-config.exe \
	-DCMAKE_MODULE_PATH=$STAGING_DIR \
	-DCMAKE_PREFIX_PATH=$STAGING_DIR/lib/cmake \
	-DCMAKE_STAGING_PREFIX=$STAGING_DIR \
	-DCMAKE_INSTALL_PREFIX=$STAGING_DIR \
)

export CMAKE="${STAGING_DIR}/bin/cmake ${CMAKE_OPTS[@]} "

AUTOCONF_OPTS="--prefix=$STAGING_DIR \
	--host=${ARCH}-w64-mingw32 \
	--enable-shared \
	--disable-static"

echo -- BUILD_STATUS_FILE:$BUILD_STATUS_FILE
echo -- MAKE_BIN:$MAKE_BIN
echo -- STAGING_DIR:$STAGING_DIR
echo -- STAGING_AREA:$STAGING_AREA
echo -- MINGW_VERSION:$MINGW_VERSION
echo -- TARGET ARCH:$ARCH
echo -- PATH:$PATH
echo -- USING CMAKE COMMAND
echo $CMAKE
echo

#!/bin/bash

if [ "$1" == "arm64" ];then
	echo "Building for aarch64"
	TOOLCHAIN_HOST="aarch64-linux-gnu"
	ARCHITECTURE=aarch64
	CMAKE_SYSTEM_PROCESSOR=aarch64
	CMAKE_LIBRARY_ARCHITECTURE=aarch64-linux-gnu
elif [ "$1" == "arm32" ]; then
	echo "Building for armhf"
	TOOLCHAIN_HOST="arm-linux-gnueabihf"
	ARCHITECTURE=armhf
	CMAKE_SYSTEM_PROCESSOR=arm
	CMAKE_LIBRARY_ARCHITECTURE=arm-linux-gnueabihf
else
	echo "$1 is invalid.  Enter first argument arm32 or arm64 to choose the toolchain"
	exit
fi

LIBSERIALPORT_BRANCH=scopy-v2
LIBIIO_VERSION=v0.26
LIBAD9361_BRANCH=main
GLOG_BRANCH=v0.4.0
LIBM2K_BRANCH=main
SPDLOG_BRANCH=v1.x
VOLK_BRANCH=main
GNURADIO_BRANCH=scopy2-maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=main
LIBSIGROKDECODE_BRANCH=master
QWT_BRANCH=qwt-multiaxes-updated
LIBTINYIIOD_BRANCH=master
IIOEMU_BRANCH=main
KDDOCK_BRANCH=2.1

export APPIMAGE=1

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
[ $CI_SCRIPT == "ON" ] && STAGING_AREA=$HOME/scopy/ci/arm/staging || STAGING_AREA=$SRC_SCRIPT/staging
SYSROOT=$STAGING_AREA/sysroot
SYSROOT_TAR=$STAGING_AREA/sysroot.tar.gz
TOOLCHAIN=$STAGING_AREA/cross-pi-gcc-$ARCHITECTURE
TOOLCHAIN_BIN=$TOOLCHAIN/bin
TOOLCHAIN_FILE=$SRC_SCRIPT/cmake_toolchain.cmake
QT_LOCATION=$SYSROOT/usr/lib/$TOOLCHAIN_HOST/qt5

CMAKE_BIN=$STAGING_AREA/cmake/bin/cmake
QMAKE_BIN=$QT_LOCATION/bin/qmake
JOBS=-j14

APP_DIR=$SRC_SCRIPT/scopy.AppDir
APP_IMAGE=$SRC_SCRIPT/Scopy.AppImage
APP_RUN=$SRC_SCRIPT/../general/AppRun
APP_DESKTOP=$SRC_SCRIPT/../general/scopy.desktop
APP_SQUASHFS=$SRC_SCRIPT/scopy.squashfs

# Runetimes downloaded from https://github.com/AppImage/AppImageKit/releases/continuous Mar 9, 2023
RUNTIME_ARM=$SRC_SCRIPT/runtime-$ARCHITECTURE


# The exports below ensure these variables are available to the toolchain file.
export CMAKE_SYSROOT="$SYSROOT"
export QT_LOCATION="$QT_LOCATION"
export STAGING_AREA="$STAGING_AREA"
export CMAKE_SYSTEM_PROCESSOR="$CMAKE_SYSTEM_PROCESSOR"
export CMAKE_LIBRARY_ARCHITECTURE="$CMAKE_LIBRARY_ARCHITECTURE"

CMAKE_OPTS=(\
	-DCMAKE_INSTALL_PREFIX="$SYSROOT" \
	-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	)

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

QT_BUILD_LOCATION=$QT_LOCATION # the location where Qt will be installed in the system
QT_SYSTEM_LOCATION=/usr/lib/$TOOLCHAIN_HOST/qt5 # the Qt location relative to the sysroot folder
CROSS_COMPILER=$TOOLCHAIN

if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
	CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Bonus%20Raspberry%20Pi%20GCC%2064-Bit%20Toolchains/Raspberry%20Pi%20GCC%2064-Bit%20Cross-Compiler%20Toolchains/Bookworm/GCC%2012.2.0/cross-gcc-12.2.0-pi_64.tar.gz
elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
	# bookwork
	#CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Bookworm/GCC%2012.2.0/Raspberry%20Pi%202%2C%203/cross-gcc-12.2.0-pi_2-3.tar.gz
	CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Bullseye/GCC%2010.2.0/Raspberry%20Pi%203A%2B%2C%203B%2B%2C%204%2C%205/cross-gcc-10.2.0-pi_3%2B.tar.gz
fi

CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz
KUIPER_DOWNLOAD_LINK=https://swdownloads.analog.com/cse/kuiper/image_2023-12-13-ADI-Kuiper-full.zip

QT_DOWNLOAD_LINK=https://download.qt.io/archive/qt/5.15/5.15.16/single/qt-everywhere-opensource-src-5.15.16.tar.xz

SYSROOT_RELATIVE_LINKS=https://raw.githubusercontent.com/abhiTronix/rpi_rootfs/master/scripts/sysroot-relativelinks.py

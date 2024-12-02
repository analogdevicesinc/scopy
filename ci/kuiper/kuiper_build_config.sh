#!/bin/bash

LIBIIO_VERSION=v0.25
LIBAD9361_BRANCH=main
GLOG_BRANCH=v0.4.0
LIBM2K_BRANCH=main
SPDLOG_BRANCH=v1.x
VOLK_BRANCH=main
GNURADIO_BRANCH=maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=main
LIBSIGROKDECODE_BRANCH=master
QWT_BRANCH=qwt-multiaxes-updated
LIBTINYIIOD_BRANCH=master
IIOEMU_BRANCH=master

STAGING_AREA=$SRC_DIR/ci/kuiper/staging
SYSROOT=$STAGING_AREA/sysroot
SYSROOT_DOCKER=$SRC_DIR/ci/kuiper/docker/sysroot
TOOLCHAIN=$STAGING_AREA/cross-pi-gcc
TOOLCHAIN_BIN=$TOOLCHAIN/bin
TOOLCHAIN_HOST="arm-linux-gnueabihf"
TOOLCHAIN_FILE=$SRC_DIR/ci/kuiper/cmake_toolchain.cmake
QT_LOCATION=$SYSROOT/usr/local/qt5.15

CMAKE_BIN=$STAGING_AREA/cmake/bin/cmake
QMAKE_BIN=$QT_LOCATION/bin/qmake
JOBS=-j14

APP_DIR=$SRC_DIR/ci/kuiper/scopy.AppDir
APP_IMAGE=$SRC_DIR/ci/kuiper/Scopy.AppImage
APP_RUN=$SRC_DIR/ci/kuiper/AppRun
APP_DESKTOP=$SRC_DIR/ci/kuiper/scopy.desktop
APP_SQUASHFS=$SRC_DIR/ci/kuiper/scopy.squashfs
RUNTIME_ARMHF=$SRC_DIR/ci/kuiper/runtime-armhf

CMAKE_OPTS=(\
	-DCMAKE_SYSROOT="$SYSROOT" \
	-DSTAGING_AREA="$STAGING_AREA" \
	-DCMAKE_INSTALL_PREFIX="$SYSROOT" \
	-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_VERBOSE_MAKEFILE=ON \
	)

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

QT_BUILD_LOCATION=$SYSROOT/usr/local/qt5.15
QT_SYSTEM_LOCATION=/usr/local/qt5.15

CROSS_COMPILER=$STAGING_AREA/cross-pi-gcc

CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Bullseye/GCC%2010.2.0/Raspberry%20Pi%203A%2B%2C%203B%2B%2C%204/cross-gcc-10.2.0-pi_3%2B.tar.gz
CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz
KUIPER_DOWNLOAD_LINK=https://swdownloads.analog.com/cse/kuiper/image_2023-12-13-ADI-Kuiper-full.zip
QT_DOWNLOAD_LINK=http://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz
SYSROOT_RELATIVE_LINKS=https://raw.githubusercontent.com/abhiTronix/rpi_rootfs/master/scripts/sysroot-relativelinks.py

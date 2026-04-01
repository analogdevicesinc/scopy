#!/bin/bash

# ARM Cross-Compilation Configuration Script
# ==========================================
# Sets up all environment variables and paths needed for ARM cross-compilation
# Usage: source arm_build_config.sh [arm32|arm64]
# Note: This script must be sourced, not executed directly


# Validate and set architecture-specific variables
# arm32: 32-bit ARM (armhf)
# arm64: 64-bit ARM (aarch64)
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


# These branches/tags define which version of each dependency to build
# Update these when you need to use different versions of dependencies
LIBSERIALPORT_BRANCH=master
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
ECM_BRANCH=kf5
KARCHIVE_BRANCH=kf5
GENALYZER_BRANCH=main

# This environment variable tells the C++ code to use relative paths for libraries
# When set to 1, the application looks for libraries in the AppImage bundle
# instead of system locations, making the package portable
export APPIMAGE=1

# SRC_SCRIPT: Directory containing this script (ci/arm)
# STAGING_AREA: Main working directory for builds
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
[ $CI_SCRIPT == "ON" ] && STAGING_AREA=$HOME/scopy/ci/arm/staging || STAGING_AREA=$SRC_SCRIPT/staging

SYSROOT=$STAGING_AREA/sysroot # root filesystem extracted from Kuiper Linux image
SYSROOT_TAR=$STAGING_AREA/sysroot.tar.gz
TOOLCHAIN=$STAGING_AREA/cross-pi-gcc # Cross-compiler location
TOOLCHAIN_BIN=$TOOLCHAIN/bin
TOOLCHAIN_FILE=$SRC_SCRIPT/cmake_toolchain.cmake

QT_LOCATION=$SYSROOT/usr/lib/$TOOLCHAIN_HOST/qt5
QT_BUILD_LOCATION=$QT_LOCATION # The location where Qt will be installed in the system
QT_SYSTEM_LOCATION=/usr/lib/$TOOLCHAIN_HOST/qt5 # The Qt location relative to the sysroot folder

CMAKE_BIN=$STAGING_AREA/cmake/bin/cmake
QMAKE_BIN=$QT_LOCATION/bin/qmake

JOBS=-j14 # Parallel build configuration

CMAKE_OPTS=(\
	-DCMAKE_INSTALL_PREFIX="$SYSROOT" \
	-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	)

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}" # Complete CMake command with all options

CROSS_COMPILER=$TOOLCHAIN

APP_DIR=$SRC_SCRIPT/scopy.AppDir # Temporary directory structure that becomes the AppImage
APP_IMAGE=$SRC_SCRIPT/Scopy.AppImage # Final AppImage executable file
APP_RUN=$SRC_SCRIPT/../general/AppRun # Entry point script that launches the application
APP_DESKTOP=$SRC_SCRIPT/../general/scopy.desktop # Desktop entry file for Linux desktop integration
APP_SQUASHFS=$SRC_SCRIPT/scopy.squashfs

# AppImage runtime for ARM
# Downloaded from https://github.com/AppImage/AppImageKit/releases/continuous
# The runtime is a small executable that mounts and runs the AppImage
RUNTIME_ARM=$SRC_SCRIPT/runtime-$ARCHITECTURE

# These exports make variables available to cmake_toolchain.cmake
# The toolchain file needs these to properly configure cross-compilation
export CMAKE_SYSROOT="$SYSROOT"
export QT_LOCATION="$QT_LOCATION"
export STAGING_AREA="$STAGING_AREA"
export CMAKE_SYSTEM_PROCESSOR="$CMAKE_SYSTEM_PROCESSOR"
export CMAKE_LIBRARY_ARCHITECTURE="$CMAKE_LIBRARY_ARCHITECTURE"

# CMake 3.29
CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz
# Qt 5.15.16 LTS source code
QT_DOWNLOAD_LINK=https://download.qt.io/archive/qt/5.15/5.15.16/single/qt-everywhere-opensource-src-5.15.16.tar.xz
# Script to fix absolute symlinks in sysroot
SYSROOT_RELATIVE_LINKS=https://raw.githubusercontent.com/abhiTronix/rpi_rootfs/master/scripts/sysroot-relativelinks.py

# Sets download URLs and Qt device specs based on target architecture
if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
	QT_BUILD_DEVICE=linux-rasp-pi4-v3d-g++
	CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Bonus%20Raspberry%20Pi%20GCC%2064-Bit%20Toolchains/Raspberry%20Pi%20GCC%2064-Bit%20Cross-Compiler%20Toolchains/Bookworm/GCC%2012.2.0/cross-gcc-12.2.0-pi_64.tar.gz
	KUIPER_DOWNLOAD_LINK=https://github.com/analogdevicesinc/adi-kuiper-gen/releases/download/v2.0.0/image_2025-04-03-ADI-Kuiper-Linux-arm64.zip
	IMAGE_NAME="2025-04-03-ADI-Kuiper-Linux-arm64"

elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
	QT_BUILD_DEVICE=linux-rasp-pi3-vc4-g++
	# bookwork
	#CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Bookworm/GCC%2012.2.0/Raspberry%20Pi%202%2C%203/cross-gcc-12.2.0-pi_2-3.tar.gz
	# bullseye, with armv8 flags
	#CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Bullseye/GCC%2010.2.0/Raspberry%20Pi%203A%2B%2C%203B%2B%2C%204%2C%205/cross-gcc-10.2.0-pi_3%2B.tar.gz
	# compiler with armv7 flags
	CROSSCOMPILER_DOWNLOAD_LINK=https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Cross-Compiler%20Toolchains/Bullseye/GCC%2010.2.0/Raspberry%20Pi%202%2C%203/cross-gcc-10.2.0-pi_2-3.tar.gz
	KUIPER_DOWNLOAD_LINK=https://swdownloads.analog.com/cse/kuiper/image_2023-12-13-ADI-Kuiper-full.zip
	IMAGE_NAME="2023-12-13-ADI-Kuiper-full"
fi

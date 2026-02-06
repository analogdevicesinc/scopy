#!/bin/bash -x

## Set STAGING
USE_STAGING=OFF
##

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || echo "No source directory found"
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$CI_SCRIPT" == "ON" ]; then
	USE_STAGING=OFF
	SRC_DIR=$GITHUB_WORKSPACE
fi

############### DEPENDECIES ###############
# DEPENDENCY="link | branch"
LIBSERIALPORT="https://github.com/sigrokproject/libserialport | master"
LIBIIO="https://github.com/analogdevicesinc/libiio | v0.26"
LIBAD9361="https://github.com/analogdevicesinc/libad9361-iio | main"
LIBM2K="https://github.com/analogdevicesinc/libm2k | main"
SPDLOG="https://github.com/gabime/spdlog | v1.14.1" # trebuie sa nu contina bug din fmt   error: no member named 'join' in namespace 'fmt'
GRSCOPY="https://github.com/analogdevicesinc/gr-scopy | 3.10"
GRM2K="https://github.com/analogdevicesinc/gr-m2k | main"
VOLK="https://github.com/cseci/volk | 2.5.1-android"
GNURADIO="https://github.com/analogdevicesinc/gnuradio | scopy2-gr3.10-android"
QWT="https://github.com/cseci/qwt | qwt-multiaxes-updated-android"
LIBSIGROKDECODE="https://github.com/sigrokproject/libsigrokdecode | master"
LIBTINYIIOD="https://github.com/analogdevicesinc/libtinyiiod | master"
IIOEMU="https://github.com/analogdevicesinc/iio-emu | main"
KDDOCK="https://github.com/KDAB/KDDockWidgets | 2.1"
GENALYZER="https://github.com/analogdevicesinc/genalyzer.git | main"
LIBSNDFILE="https://github.com/libsndfile/libsndfile | 1.2.0" # mai nou ca in scopy-android-deps
PYTHON="https://github.com/python/cpython | 3.12" # acelasi ca versiunea instalata local, mai trebuie adaugate modificari specifice
GLIB="https://github.com/GNOME/glib | 2.85.4" # mai nou ca in scopy-android-deps
ANDROIDBOOST="https://github.com/moritz-wundke/Boost-for-Android | master" # mai nou ca in scopy-android-deps
LIBXML2="https://github.com/GNOME/libxml2 | v2.9.13" # mai nou ca in scopy-android-deps
LIBICONV="https://github.com/roboticslibrary/libiconv | v1.17" # mai nou ca in scopy-android-deps
GETTEXT="https://github.com/autotools-mirror/gettext | v0.21.1" # mai nou ca in scopy-android-deps
FFTW="https://github.com/FFTW/fftw3| fftw-3.3.10" # mai nou ca in scopy-android-deps (PENTRU GENALYZER)
LIBGMP="https://github.com/bastibl/libgmp | master"
LIBUSB="https://github.com/jagheterfredrik/libusb | android-rebase-2022-07" # trebuie folosit branchul de mai jos, dar cu doua commituri de aici
# LIBUSB="https://github.com/CraigHutchinson/libusb | android-rebase-2022-07" # mai nou ca in scopy-android-deps (ultima versiune de libusb pt android existenta)
LIBZMQ="https://github.com/zeromq/libzmq | v4.3.5"
LIBFFI="https://github.com/libffi/libffi | v3.3" # mai nou ca in scopy-android-deps
GNULIB="https://github.com/coreutils/gnulib | v1.0"
LIBMATIO="https://github.com/tbeu/matio | master"
ECM="https://github.com/KDE/extra-cmake-modules.git | kf5"
KARCHIVE="https://github.com/KDE/karchive.git | kf5"
LIBZSTD="https://github.com/pexip/os-libzstd | bookworm"


# De Adaugat: https://github.com/fmtlib/fmt ????
# FMT="https://github.com/fmtlib/fmt | master"

############### ANDROID CONFIGURATION ###############
# NDK_VERSION=27.2.12479018
# NDK_VERSION=26.1.10909125
NDK_VERSION=25.2.9519653
# NDK_VERSION=23.2.8568313
API=33
export APP_PLATFORM=$API
# ANDROID_SDK_BUILD_TOOLS=33.0.1
ANDROID_SDK_BUILD_TOOLS=35.0.1
HOST_ARCH=linux-x86_64
ARCH=arm64
ABI=arm64-v8a
TARGET=aarch64-linux-android 
TARGET_PREFIX=$TARGET
TARGET_PROCESSOR=aarch64

###############  ###############
STAGING_AREA=$SRC_SCRIPT/staging
STAGING_AREA_DEPS=$STAGING_AREA/dependencies
export ANDROID_SDK_ROOT=$STAGING_AREA/sdk
export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION
export ANDROID_NDK=$ANDROID_NDK_ROOT
TOOLCHAIN_BIN=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/$HOST_ARCH/bin
CMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake
TOOLCHAIN=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/$HOST_ARCH
SYSROOT=$TOOLCHAIN/sysroot

QT_VERSION_STRING=5.15.17
###
#QT_LOCATION=/opt/Qt/$QT_VERSION_STRING/android
#QT_LOCATION=$STAGING_AREA/$QT_VERSION_STRING/android
QT_LOCATION=/media/cristian/ssd/Qt5-kde-android-arm64
export QT_INSTALL_PREFIX=$QT_LOCATION
###

QMAKE=$QT_LOCATION/bin/qmake
CMAKE_BIN=$STAGING_AREA/cmake/bin/cmake

JOBS=-j14

BUILD_STATUS_FILE=$SRC_SCRIPT/build-status
SDK_MANAGER=$STAGING_AREA/tools/bin/sdkmanager

############### COMPILER FLAGS ###############
# #CC=$TOOLCHAIN_BIN/clang ??
CC=$TOOLCHAIN_BIN/$TARGET_PREFIX$API-clang
CXX=$TOOLCHAIN_BIN/$TARGET_PREFIX$API-clang++
CPP="$CC -E"
LD=$TOOLCHAIN_BIN/ld.lld
AS=${CC}
AR=$TOOLCHAIN_BIN/llvm-ar
RANLIB=$TOOLCHAIN_BIN/llvm-ranlib
NM=$TOOLCHAIN_BIN/llvm-nm
STRIP=$TOOLCHAIN_BIN/llvm-strip
READELF=$TOOLCHAIN_BIN/llvm-readelf
STRIP=$TOOLCHAIN_BIN/llvm-strip

# ############### SYSTEM SPECIFIC DEFINES ###############

export JAVA_HOME=$STAGING_AREA/jdk
export PATH=${TOOLCHAIN_BIN}:$JAVA_HOME/bin:${PATH}
# # JDK=/usr/lib/jvm/java-16-openjdk-amd64
# PYTHON_VERSION=3.8.10
# SCRIPT_HOME_DIR=$HOME/src/scopy-android-deps
# DEPS_SRC_PATH=$SCRIPT_HOME_DIR/downloads
# BUILD_ROOT=$SCRIPT_HOME_DIR/gnuradio-android
# ANDROID_QT_DEPLOY=$QT_INSTALL_PREFIX/bin/androiddeployqt
# #PREFIX=${BUILD_ROOT}/toolchain/$ABI




export CFLAGS="-I${SYSROOT}/include -I${SYSROOT}/usr/include -I${SYSROOT}/usr/include/android -I${TOOLCHAIN}/include -I${STAGING_AREA_DEPS}/include -fPIC"
export CPPFLAGS="-fexceptions -frtti ${CFLAGS}"

# !!!!!!!! https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md#Unwinding
# TBD Which libraries should link unwind manually
#-lunwind # de vazut la care trebe
export LDFLAGS="-avoid-version -L${STAGING_AREA_DEPS} -L${STAGING_AREA_DEPS}/lib -L${STAGING_AREA_DEPS}/usr/lib/$TARGET_PREFIX/$API -L${TOOLCHAIN}/lib -lunwind"


echo ANDROID_SDK=$ANDROID_SDK_ROOT
echo CMAKE=$CMAKE
echo QT_LOCATION=$QT_LOCATION
echo JAVA_HOME=$JAVA_HOME
echo NDK_VERSION=$NDK_VERSION
echo JOBS=$JOBS
echo SCRIPT_HOME_DIR=$SCRIPT_HOME_DIR
echo TARGET=$TARGET
echo HOST=$HOST
echo ARCH=$ARCH


CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz

set_cmake_opts(){

	# BUILD_TYPE=Release
	BUILD_TYPE=Debug
	# BUILD_TYPE=RelWithDebInfo

	CMAKE_OPTS=(\
		-DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
		-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
		-DCMAKE_INSTALL_PREFIX="$STAGING_AREA_DEPS" \
		-DCMAKE_FIND_ROOT_PATH="${QT_LOCATION}" \
		-DCMAKE_LIBRARY_PATH="${STAGING_AREA_DEPS}" \
		-DCMAKE_STAGING_PREFIX="${STAGING_AREA_DEPS}" \
		-DCMAKE_PREFIX_PATH="${STAGING_AREA_DEPS}/lib/cmake\;${QT_LOCATION}/lib/cmake;${QT_LOCATION}" \
		-DQT_QMAKE_EXECUTABLE="${QT_LOCATION}/bin/qmake" \
		-DCMAKE_SYSROOT="$SYSROOT" \
		-DCMAKE_ASM_COMPILER="${TOOLCHAIN_BIN}/${TARGET}${API}-clang" \
		-DCMAKE_C_COMPILER="${TOOLCHAIN_BIN}/${TARGET}${API}-clang" \
		-DCMAKE_CXX_COMPILER="${TOOLCHAIN_BIN}/${TARGET}${API}-clang++" \
		-DCMAKE_LINKER="${TOOLCHAIN_BIN}/ld" \
		-DCMAKE_AR="${TOOLCHAIN_BIN}/llvm-ar" \
		-DCMAKE_OBJCOPY="${TOOLCHAIN_BIN}/llvm-objcopy" \
		-DCMAKE_RANLIB="${TOOLCHAIN_BIN}/llvm-ranlib" \
		-DCMAKE_SIZE="${TOOLCHAIN_BIN}/llvm-size" \
		-DCMAKE_STRIP="${TOOLCHAIN_BIN}/llvm-strip" \
		-DCMAKE_SYSTEM_NAME="Android" \
		-DCMAKE_SYSTEM_VERSION="$API" \
		-DQT_ANDROID=ON \
		-DQT_DIR="${QT_LOCATION}" \
		-DQt5_DIR="${QT_LOCATION}" \
		-DANDROID_ABI="$ABI" \
		-DANDROID_API_VERSION="$API" \
		-DANDROID_ARM_NEON="ON" \
		-DANDROID_LD="lld" \
		-DANDROID_STL="c++_shared" \
		-DCMAKE_ANDROID_STL_TYPE="c++_shared" \
		-DANDROID_NATIVE_API_LEVEL="${API}" \
		-DANDROID_NDK="$ANDROID_NDK_ROOT" \
		-DANDROID_NDK_ROOT="${ANDROID_NDK}" \
		-DCMAKE_ANDROID_NDK="${ANDROID_NDK}" \
		-DANDROID_PLATFORM="android-${API}" \
		-DCMAKE_ANDROID_ARCH_ABI="$ABI" \
		-DANDROID_TOOLCHAIN="clang" \
		-DANDROID_SDK="$ANDROID_SDK_ROOT" \
		-DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
		-DANDROID_SDK_BUILD_TOOLS_REVISION="$ANDROID_SDK_BUILD_TOOLS" \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_VERBOSE=ON \
	)

	# Debug Mode
	#       -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS} -Wl,--verbose" \
	# 	-DCMAKE_VERBOSE=ON \
	# 	-DCMAKE_VERBOSE_MAKEFILE=ON 
	# 	-DPKG_CONFIG_ARGN="--debug"\
	# 	-DCMAKE_FIND_DEBUG_MODE=TRUE \

	CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"
}

set_configure_opts(){

	CONFIG_OPTS=()
	CONFIG_OPTS+=("--build=x86_64-unknown-linux-gnu")
	CONFIG_OPTS+=("--host=$TARGET_PREFIX")
	CONFIG_OPTS+=("--prefix=${STAGING_AREA_DEPS}")
	# CONFIG_OPTS+=("--with-sysroot=${SYSROOT}")
	CONFIG_OPTS+=("PKG_CONFIG_ALLOW_CROSS=1")
	CONFIG_OPTS+=("PKG_CONFIG_PATH=${STAGING_AREA_DEPS}/lib/pkgconfig") # Don't mix up .pc files from your host and build target
	CONFIG_OPTS+=("LDFLAGS=${LDFLAGS}")
	CONFIG_OPTS+=("CFLAGS=${CFLAGS}")
	CONFIG_OPTS+=("CPPFLAGS=${CPPFLAGS}")
	CONFIG_OPTS+=("CPP=${CPP}")
	CONFIG_OPTS+=("CC=${CC}")
	CONFIG_OPTS+=("CXX=${CXX}")
	CONFIG_OPTS+=("LD=${LD}")
	CONFIG_OPTS+=("AS=${AS}")
	CONFIG_OPTS+=("AR=${AR}")
	CONFIG_OPTS+=("RANLIB=${RANLIB}")
	CONFIG_OPTS+=("READELF=${READELF}")
	CONFIG_OPTS+=("NM=${NM}")
	CONFIG_OPTS+=("STRIP=${STRIP}")
}




#!/bin/bash -e

# Scopy Android (Qt6) toolchain configuration
# ===========================================
# Sourced by android_deps_build.sh; it is not meant to be executed directly.
#
# ABI-parameterised: export ABI=x86_64 or ABI=arm64-v8a before sourcing. Each ABI
# gets its own staging prefix, so both can be built side by side. Every location
# is env-overridable so the same file serves the local dev loop (Qt in ~/Qt) and
# the CI container (Qt in /opt/Qt).

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

[ "$CI_SCRIPT" == "ON" ] && SRC_DIR=$GITHUB_WORKSPACE

############### DEPENDENCIES ###############
# Pins track ci/ubuntu/ubuntu_build_process.sh so Android does not silently
# diverge from the desktop builds. Only the deps the slim build actually needs
# are listed: M2K is force-disabled (packages/CMakeLists.txt) and ENABLE_PLUGIN_ADC
# defaults OFF, which skips gr-util, so gnuradio, volk, genalyzer, boost, matio,
# libsigrokdecode, glib and python are all out of scope.
#
# libserialport is NOT optional: iioutil/src/iioscantask.cpp includes
# libserialport.h and calls sp_* unconditionally, so Scopy does not build without
# it on any platform.
LIBSERIALPORT_BRANCH=master
LIBXML2_BRANCH=v2.13.8
LIBIIO_VERSION=v1.0.0
LIBIIO_V0_VERSION=v0.26
ZSTD_BRANCH=v1.5.7
LIBAD9361_BRANCH=main
QWT_BRANCH=qwt-multiaxes-rebased-qt6
ECM_BRANCH=v6.8.0
KARCHIVE_BRANCH=v6.8.0
QCORO_BRANCH=v0.13.0
KDDOCK_BRANCH=2.2

############### ANDROID CONFIGURATION ###############
# NDK r26b is what Qt 6.8 documents and what Qt's own doc examples use.
NDK_VERSION=${NDK_VERSION:-26.1.10909125}
# Qt 6.8's androiddeployqt hard-floors minSdkVersion at 28; build the deps for the
# same level so they load on every device the APK targets.
API=${API:-28}
HOST_ARCH=${HOST_ARCH:-linux-x86_64}

ABI=${ABI:-arm64-v8a}
case "$ABI" in
arm64-v8a)
	ANDROID_TARGET=aarch64-linux-android
	CONFIGURE_HOST=aarch64-linux-android
	;;
x86_64)
	ANDROID_TARGET=x86_64-linux-android
	CONFIGURE_HOST=x86_64-linux-android
	;;
*)
	echo "ERROR: unsupported ABI '$ABI' (expected arm64-v8a or x86_64)" >&2
	return 1 2>/dev/null || exit 1
	;;
esac

export ANDROID_SDK_ROOT=${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$HOME/Android/Sdk}}
export ANDROID_NDK_ROOT=${ANDROID_NDK_ROOT:-$ANDROID_SDK_ROOT/ndk/$NDK_VERSION}
export ANDROID_NDK=$ANDROID_NDK_ROOT

############### ANDROID SDK COMPONENTS ###############
# One platform and one build-tools revision only; androiddeployqt always picks the highest.
# ANDROID_SDK_PLATFORM must match QT_ANDROID_TARGET_SDK_VERSION in CMakeLists.txt.
ANDROID_SDK_PLATFORM=${ANDROID_SDK_PLATFORM:-34}
ANDROID_SDK_BUILD_TOOLS=${ANDROID_SDK_BUILD_TOOLS:-34.0.0}
ANDROID_CMDLINE_TOOLS=${ANDROID_CMDLINE_TOOLS:-11076708}
SDKMANAGER=$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager

TOOLCHAIN=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/$HOST_ARCH
TOOLCHAIN_BIN=$TOOLCHAIN/bin
SYSROOT=$TOOLCHAIN/sysroot
CMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake

############### QT ###############
QT_VERSION=${QT_VERSION:-6.8.3}
QT_INSTALL_LOCATION=${QT_INSTALL_LOCATION:-/opt/Qt}
QT_ANDROID=${QT_ANDROID:-$QT_INSTALL_LOCATION/$QT_VERSION/android_${ABI//-/_}}
# Cross-building Qt needs host moc/rcc/uic/androiddeployqt; Qt's CMake reads this
# as QT_HOST_PATH.
QT_HOST_PATH=${QT_HOST_PATH:-$QT_INSTALL_LOCATION/$QT_VERSION/gcc_64}
QMAKE_BIN=$QT_ANDROID/bin/qmake6

############### STAGING ###############
STAGING_AREA=${STAGING_AREA:-$SRC_SCRIPT/staging_android}
STAGING_AREA_SRC=$STAGING_AREA/src
STAGING_AREA_DEPS=$STAGING_AREA/$ABI/dependencies

############### LIBIIO ###############
# libiio API Scopy is built against: 1 (default) or 0. Same image serves both: libiio v0 and
# libad9361 (built on v0) live in their own prefix, as in ci/x86_64/x86-64_appimage_process.sh.
SCOPY_LIBIIO_VERSION=${SCOPY_LIBIIO_VERSION:-1}
LIBIIO_V0_PREFIX=$STAGING_AREA_DEPS/libiio-v0

############### SIGNING ###############
# An unsigned APK cannot be installed, so the build always signs. Defaults are Android's
# conventional debug values; override to sign with a real key.
ANDROID_KEYSTORE=${ANDROID_KEYSTORE:-}
ANDROID_KEYSTORE_ALIAS=${ANDROID_KEYSTORE_ALIAS:-androiddebugkey}
ANDROID_KEYSTORE_PASS=${ANDROID_KEYSTORE_PASS:-android}

JOBS=${JOBS:--j$(nproc)}
CMAKE_BIN=${CMAKE_BIN:-$(command -v cmake)}

############### COMPILER / LINKER FLAGS ###############
# Deliberately NOT set, and why:
#   -avoid-version      a libtool flag; clang rejects it ("unknown argument").
#                       Unversioned SONAMEs are handled by
#                       CMAKE_PLATFORM_NO_VERSIONED_SONAME below instead.
#   -L${TOOLCHAIN}/lib  that is the NDK's HOST (x86_64) lib dir; adding it makes
#                       the link fail with "libc++.so is incompatible". The NDK
#                       supplies the correct per-ABI libc++/unwind from the
#                       sysroot automatically.
export CFLAGS="-I${STAGING_AREA_DEPS}/include -fPIC"
export CPPFLAGS="-fexceptions -frtti ${CFLAGS}"
# NDK r26 makes --no-undefined-version fatal, which breaks any project whose
# version script lists symbols excluded by our feature flags (libxml2 does).
export LDFLAGS="-L${STAGING_AREA_DEPS}/lib -Wl,--undefined-version"

# pkg-config shells out, so it ignores CMAKE_FIND_ROOT_PATH and will happily
# report a HOST library whose headers do not exist in the NDK sysroot. KArchive
# does exactly that: pkg_check_modules(LibZstd) finds the host libzstd, enables
# kzstdfilter.cpp and then fails on a missing zstd.h - and note its WITH_LIBZSTD
# option only controls whether zstd is *required*, not whether it is used.
# PKG_CONFIG_LIBDIR *replaces* the default search path; PKG_CONFIG_PATH only
# prepends to it, which would leave the host .pc files visible.
export PKG_CONFIG_LIBDIR=$STAGING_AREA_DEPS/lib/pkgconfig:$SYSROOT/usr/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$SYSROOT

CC=$TOOLCHAIN_BIN/${ANDROID_TARGET}${API}-clang
CXX=$TOOLCHAIN_BIN/${ANDROID_TARGET}${API}-clang++
CPP="$CC -E"
AR=$TOOLCHAIN_BIN/llvm-ar
RANLIB=$TOOLCHAIN_BIN/llvm-ranlib
NM=$TOOLCHAIN_BIN/llvm-nm
STRIP=$TOOLCHAIN_BIN/llvm-strip
READELF=$TOOLCHAIN_BIN/llvm-readelf
LD=$TOOLCHAIN_BIN/ld.lld
AS=$CC

set_cmake_opts() {
	BUILD_TYPE=${BUILD_TYPE:-Release}

	# Android only loads lib*.so from the APK's lib/<abi>/ dir, flat and
	# UNVERIFIED. androiddeployqt enforces this: copyAndroidExtraLibs() hard-fails
	# on any entry not matching lib*.so, so a libfoo.so.1 dep is unusable.
	#   CMAKE_PLATFORM_NO_VERSIONED_SONAME -> libfoo.so, not libfoo.so.1
	#   CMAKE_INSTALL_LIBDIR=lib           -> no lib/<triple>/ multiarch subdir
	#                                         (KArchive installs there otherwise)
	CMAKE_OPTS=(
		-DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE"
		-DCMAKE_BUILD_TYPE="$BUILD_TYPE"
		-DCMAKE_INSTALL_PREFIX="$STAGING_AREA_DEPS"
		-DCMAKE_INSTALL_LIBDIR=lib
		-DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON
		-DCMAKE_PREFIX_PATH="${STAGING_AREA_DEPS};${QT_ANDROID}"
		-DCMAKE_FIND_ROOT_PATH="${STAGING_AREA_DEPS};${QT_ANDROID};${SYSROOT}"
		-DQT_HOST_PATH="$QT_HOST_PATH"
		-DANDROID_ABI="$ABI"
		-DANDROID_PLATFORM="android-${API}"
		-DANDROID_NDK="$ANDROID_NDK_ROOT"
		-DANDROID_STL=c++_shared
		-DANDROID_LD=lld
		-DCMAKE_MAKE_PROGRAM="$(command -v make)"
		-DCMAKE_C_FLAGS="$CFLAGS"
		-DCMAKE_CXX_FLAGS="$CPPFLAGS"
		-DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS"
		-DBUILD_TESTING=OFF
	)
}

set_configure_opts() {
	CONFIG_OPTS=(
		"--build=x86_64-unknown-linux-gnu"
		"--host=$CONFIGURE_HOST"
		"--prefix=$STAGING_AREA_DEPS"
		"PKG_CONFIG_ALLOW_CROSS=1"
		"PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR"
		"PKG_CONFIG_SYSROOT_DIR=$SYSROOT"
		"CFLAGS=$CFLAGS"
		"CPPFLAGS=$CPPFLAGS"
		"LDFLAGS=$LDFLAGS"
		"CC=$CC"
		"CXX=$CXX"
		"CPP=$CPP"
		"LD=$LD"
		"AS=$AS"
		"AR=$AR"
		"RANLIB=$RANLIB"
		"NM=$NM"
		"STRIP=$STRIP"
		"READELF=$READELF"
	)
}

# androiddeployqt always selects the HIGHEST installed SDK platform and build-tools
# and offers no override (not even from CMake). Qt 6.8.3 ships Android Gradle Plugin
# 8.6.0, which supports compileSdk up to 35, so an SDK containing android-36 or
# newer makes Gradle fail late with an opaque AGP error. Catch it here instead.
MAX_SUPPORTED_SDK_PLATFORM=${MAX_SUPPORTED_SDK_PLATFORM:-35}

check_sdk_platforms() {
	local dir=$ANDROID_SDK_ROOT/platforms
	[ -d "$dir" ] || {
		echo "ERROR: no platforms/ in ANDROID_SDK_ROOT ($ANDROID_SDK_ROOT)" >&2
		return 1
	}

	local highest
	highest=$(ls -1 "$dir" | sed -n 's/^android-\([0-9]\+\)$/\1/p' | sort -n | tail -1)
	[ -n "$highest" ] || {
		echo "ERROR: no android-<N> platform found in $dir" >&2
		return 1
	}

	if [ "$highest" -gt "$MAX_SUPPORTED_SDK_PLATFORM" ]; then
		cat >&2 <<-EOF

		ERROR: $ANDROID_SDK_ROOT exposes android-$highest.
		       androiddeployqt picks the highest installed platform and cannot be
		       overridden, but the Android Gradle Plugin shipped with Qt $QT_VERSION
		       supports at most compileSdk $MAX_SUPPORTED_SDK_PLATFORM, so the Gradle
		       step would fail.

		       Point ANDROID_SDK_ROOT at an SDK exposing no platform newer than
		       $MAX_SUPPORTED_SDK_PLATFORM. A directory of symlinks is enough, and
		       leaves your main SDK untouched for Android Studio:

		         ALT=\$HOME/Android/Sdk-qt6
		         mkdir -p \$ALT/platforms \$ALT/build-tools
		         for d in ndk platform-tools cmdline-tools licenses emulator; do
		           ln -sfn "$ANDROID_SDK_ROOT/\$d" "\$ALT/\$d"
		         done
		         P=android-$MAX_SUPPORTED_SDK_PLATFORM
		         ln -sfn "$ANDROID_SDK_ROOT/platforms/\$P" "\$ALT/platforms/\$P"
		         ln -sfn "$ANDROID_SDK_ROOT/build-tools/<rev>" "\$ALT/build-tools/<rev>"
		         export ANDROID_SDK_ROOT=\$ALT

		EOF
		return 1
	fi
	echo "SDK platform check: highest is android-$highest (max $MAX_SUPPORTED_SDK_PLATFORM) - ok"
}

print_config() {
	check_sdk_platforms
	cat <<-EOF
	--- Scopy Android toolchain ---
	ABI                 $ABI
	ANDROID_TARGET      $ANDROID_TARGET
	API                 $API
	NDK                 $ANDROID_NDK_ROOT
	ANDROID_SDK_ROOT    $ANDROID_SDK_ROOT
	QT_ANDROID          $QT_ANDROID
	QT_HOST_PATH        $QT_HOST_PATH
	STAGING_AREA_DEPS   $STAGING_AREA_DEPS
	LIBIIO_V0_PREFIX    $LIBIIO_V0_PREFIX
	SCOPY_LIBIIO        v$SCOPY_LIBIIO_VERSION
	JOBS                $JOBS
	-------------------------------
	EOF
}

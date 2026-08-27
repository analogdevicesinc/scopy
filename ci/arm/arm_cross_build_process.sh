#!/bin/bash -xe

# Cross-compilation Build Process for Scopy armhf (Qt6)
# =====================================================
# Runs on x86_64 host, cross-compiles for ARM32 hard-float
# Uses Kuiper Linux sysroot (created by create_sysroot_armhf.sh)
# Usage: ./arm_cross_build_process.sh [function_name ...]

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null) || \
SRC_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && cd ../../ && pwd)

SRC_SCRIPT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

ARCH_SUFFIX=armhf
ARCHITECTURE=armhf
TOOLCHAIN_HOST=arm-linux-gnueabihf

LIBSERIALPORT_BRANCH=master
LIBIIO_VERSION=v0.26
LIBAD9361_BRANCH=main
LIBAD9166_BRANCH=libad9166-iio-v0
QWT_BRANCH=qwt-multiaxes-updated
LIBTINYIIOD_BRANCH=master
IIOEMU_BRANCH=main
KDDOCK_BRANCH=2.2
ECM_BRANCH=v6.8.0
KARCHIVE_BRANCH=v6.8.0
GENALYZER_BRANCH=main

export APPIMAGE=1

STAGING_AREA=$SRC_SCRIPT/staging
SYSROOT=/opt/sysroot/armhf
QT_INSTALL_PREFIX=/opt/Qt/6.8.3/armhf
QT=$QT_INSTALL_PREFIX
JOBS=-j$(nproc)

PYTHON_VERSION=$(python3 -c 'import sys; print(f"python3.{sys.version_info.minor}")' 2>/dev/null || echo "python3")

CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz
CMAKE_BIN=${STAGING_AREA}/cmake/bin/cmake
# The `|| true` is required. `command -v` exits 1 when cmake is not installed, and this script runs
# under `set -e`, so the bare assignment aborted the entire script on this line - in the script's
# preamble, before any function could run. That is fatal to the image build:
# Dockerfile.armhf-cross runs `install_packages` as its first script invocation, and install_packages
# is itself what installs cmake, so cmake is guaranteed absent on that pass.
# The fallback is still needed at CI time: the Dockerfile deletes /home/runner/scripts, so
# $STAGING_AREA/cmake does not exist in the finished image and the system cmake is the only one left.
# `command -v` rather than `which`: it is a shell builtin, needs no package, and matches
# arm_native_build_process.sh, which keeps the same fallback inside download_cmake().
[ ! -f "$CMAKE_BIN" ] && CMAKE_BIN=$(command -v cmake || true)

APP_DIR=$SRC_SCRIPT/scopy.AppDir
APP_IMAGE=$SRC_SCRIPT/Scopy.AppImage
APP_RUN=$SRC_SCRIPT/../general/AppRun
APP_DESKTOP=$SRC_SCRIPT/../general/scopy.desktop
APP_SQUASHFS=$SRC_SCRIPT/scopy.squashfs
RUNTIME_ARM=$SRC_SCRIPT/runtime-$ARCHITECTURE

TOOLCHAIN_FILE=$SRC_SCRIPT/cmake_toolchain_armhf.cmake

export CMAKE_SYSROOT=$SYSROOT
export QT_LOCATION=$QT

export CC=$TOOLCHAIN_HOST-gcc
export CXX=$TOOLCHAIN_HOST-g++
export AR=$TOOLCHAIN_HOST-ar
export LD=$TOOLCHAIN_HOST-ld
export STRIP=$TOOLCHAIN_HOST-strip

export PKG_CONFIG_ALLOW_CROSS=1
export PKG_CONFIG_SYSROOT_DIR=$SYSROOT
export PKG_CONFIG_LIBDIR=$SYSROOT/usr/lib/$TOOLCHAIN_HOST/pkgconfig:$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig:$SYSROOT/usr/local/lib/pkgconfig

CMAKE_OPTS=(\
	-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_VERBOSE_MAKEFILE=ON \
	-DCMAKE_INSTALL_PREFIX=$SYSROOT \
)

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

echo "-- USING CMAKE COMMAND: $CMAKE"
echo "-- USING QT: $QT"
echo "-- SYSROOT: $SYSROOT"
echo "-- TOOLCHAIN_FILE: $TOOLCHAIN_FILE"

download_cmake() {
	echo "#######DOWNLOAD CMAKE#######"
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d cmake ]; then
		wget --progress=dot:giga ${CMAKE_DOWNLOAD_LINK}
		tar -xf cmake*.tar.gz && rm cmake*.tar.gz && mv cmake* cmake
	else
		echo "Cmake already downloaded"
	fi
	popd
}

install_packages() {
	# Dropped with the slim dependency set: swig (gnuradio's Python bindings) and libglib2.0-dev.
	# Removing libglib2.0-dev from this list is cosmetic, not a reduction - libinput-dev (kept
	# below, needed by Qt6) pulls it back transitively via libwacom-dev / libgudev-1.0-dev.
	# Verified on Ubuntu 24.04 in this pass and on Debian 13 during the arm64 pass.
	sudo apt-get update
	sudo apt-get -y upgrade
	sudo apt-get -y install \
		build-essential cmake git wget ca-certificates unzip \
		g++ pkg-config flex bison autogen autoconf autoconf-archive libtool \
		crossbuild-essential-armhf \
		ninja-build patchelf squashfs-tools \
		python3 python3-pip python3-dev python3-numpy python3-packaging python3-mako \
		libgl1-mesa-dev libglu1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev \
		libxcb1-dev libxcb-xinerama0 libxcb-xinerama0-dev libxcb-icccm4-dev \
		libxcb-xfixes0-dev libxcb-render0-dev \
		libxcb-keysyms1-dev libxcb-shape0-dev libxcb-xkb-dev \
		libxcb-cursor-dev libxcb-image0-dev libxcb-render-util0-dev \
		libxcb-sync-dev libxcb-randr0-dev libxcb-shm0-dev \
		libxcb-util-dev libxcb-ewmh-dev \
		libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev \
		libxrender-dev libxcomposite-dev \
		libdrm-dev libgbm-dev libinput-dev libudev-dev \
		libssl-dev libjpeg-dev libpng-dev \
		libdouble-conversion-dev \
		libzstd-dev libbz2-dev liblzma-dev \
		libfontconfig1-dev libfreetype-dev libpcre2-dev libharfbuzz-dev
}

set_config_opts() {
	CONFIG_OPTS=()
	CONFIG_OPTS+=("--prefix=${SYSROOT}")
	CONFIG_OPTS+=("--host=${TOOLCHAIN_HOST}")
	CONFIG_OPTS+=("--with-sysroot=${SYSROOT}")
	CONFIG_OPTS+=("PKG_CONFIG_DIR=")
	CONFIG_OPTS+=("PKG_CONFIG_LIBDIR=${SYSROOT}/usr/lib/${TOOLCHAIN_HOST}/pkgconfig:${SYSROOT}/usr/share/pkgconfig:${SYSROOT}/usr/local/lib/pkgconfig")
	CONFIG_OPTS+=("PKG_CONFIG_SYSROOT_DIR=${SYSROOT}")
	CONFIG_OPTS+=("PKG_CONFIG_ALLOW_CROSS=1")
	CONFIG_OPTS+=("CC=${TOOLCHAIN_HOST}-gcc")
	CONFIG_OPTS+=("CXX=${TOOLCHAIN_HOST}-g++")
	CONFIG_OPTS+=("LD=${TOOLCHAIN_HOST}-ld")
	CONFIG_OPTS+=("AR=${TOOLCHAIN_HOST}-ar")
	CONFIG_OPTS+=("RANLIB=${TOOLCHAIN_HOST}-ranlib")

	local CFLAGS="-march=armv7-a -mfloat-abi=hard -mfpu=neon"
	CFLAGS="${CFLAGS} -I${SYSROOT}/include -I${SYSROOT}/usr/include -I${SYSROOT}/usr/include/${TOOLCHAIN_HOST} -fPIC"
	CONFIG_OPTS+=("CFLAGS=${CFLAGS}")
	CONFIG_OPTS+=("CPPFLAGS=-fexceptions ${CFLAGS}")
	CONFIG_OPTS+=("LDFLAGS=--sysroot=${SYSROOT} -L${SYSROOT}/lib -L${SYSROOT}/usr/lib -L${SYSROOT}/usr/lib/${TOOLCHAIN_HOST}")
}

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	[ -d 'libserialport' ] || git clone --recursive https://github.com/sigrokproject/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	[ -d 'libiio' ]		|| git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	[ -d 'libad9361' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	[ -d 'libad9166' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9166-iio.git -b $LIBAD9166_BRANCH libad9166
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'libtinyiiod' ]	|| git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	[ -d 'iio-emu' ]	|| git clone --recursive https://github.com/analogdevicesinc/iio-emu -b $IIOEMU_BRANCH iio-emu
	[ -d 'KDDockWidgets' ] || git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	[ -d 'extra-cmake-modules' ] || git clone --recursive https://github.com/KDE/extra-cmake-modules.git -b $ECM_BRANCH extra-cmake-modules
	[ -d 'karchive' ] || git clone --recursive https://github.com/KDE/karchive.git -b $KARCHIVE_BRANCH karchive
	[ -d 'genalyzer' ] || git clone --recursive https://github.com/analogdevicesinc/genalyzer.git -b $GENALYZER_BRANCH genalyzer
	popd
}

build_with_cmake() {
	download_cmake
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	BUILD_FOLDER=$PWD/build
	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $CURRENT_BUILD_CMAKE_OPTS ../
	make $JOBS
	if [ "$INSTALL" == "ON" ]; then
		sudo make install
	fi
	CURRENT_BUILD_CMAKE_OPTS=""
}

build_libserialport() {
	echo "### Building libserialport - branch $LIBSERIALPORT_BRANCH"
	set_config_opts
	pushd $STAGING_AREA/libserialport
	git clean -xdf
	./autogen.sh
	./configure "${CONFIG_OPTS[@]}"
	make $JOBS
	[ "$1" == "ON" ] && sudo make install
	popd
}

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"
	pushd $STAGING_AREA/libiio
	CURRENT_BUILD_CMAKE_OPTS="\
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_DOC:BOOL=OFF \
		-DHAVE_DNS_SD:BOOL=OFF \
		-DWITH_MATLAB_BINDINGS:BOOL=OFF \
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		-DWITH_SERIAL_BACKEND:BOOL=ON \
		-DENABLE_IPV6:BOOL=OFF \
		-DINSTALL_UDEV_RULE:BOOL=OFF
		"
	build_with_cmake $1
	popd
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	pushd $STAGING_AREA/libad9361
	build_with_cmake $1
	popd
}

build_libad9166() {
	echo "### Building libad9166 - branch $LIBAD9166_BRANCH"
	pushd $STAGING_AREA/libad9166
	build_with_cmake $1
	popd
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	pushd $STAGING_AREA/qwt
	git clean -xdf
	sed -i 's|/usr/local/qwt-$$QWT_VERSION-ma|/usr/local|g' qwtconfig.pri
	# Rename the produced library base name to qwt_scopy (and its SONAME in lockstep)
	sed -i 's|qwtLibraryTarget(qwt)|qwtLibraryTarget(qwt_scopy)|' src/src.pro
	sed -i 's|QWT_SONAME=libqwt.so|QWT_SONAME=libqwt_scopy.so|' src/src.pro
	# The designer/examples/playground/tests subprojects link the library by its
	# old name (qwtAddLibrary(..., qwt)); update those to match the rename above,
	# otherwise they fail with `ld: cannot find -lqwt`. (Gap in PR #2291.)
	sed -i 's|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt)|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt_scopy)|' \
		designer/designer.pro examples/examples.pri playground/playground.pri tests/tests.pri
	$QT/bin/qmake6 INCLUDEPATH=$SYSROOT/include LIBS=-L$SYSROOT/lib LIBS+=-L$SYSROOT/lib/$TOOLCHAIN_HOST qwt.pro
	make $JOBS
	sudo make INSTALL_ROOT=$SYSROOT install
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	pushd $STAGING_AREA/libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	build_with_cmake $1
	popd
}

build_kddock() {
	echo "### Building KDDockWidgets - version $KDDOCK_BRANCH"
	pushd $STAGING_AREA/KDDockWidgets
	CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON -DKDDockWidgets_FRONTENDS=qtwidgets"
	build_with_cmake $1
	# D18: KDDock installs headers to $SYSROOT/include/ but cmake expects $SYSROOT/usr/include/
	[ -d $SYSROOT/include/kddockwidgets-qt6 ] && [ ! -e $SYSROOT/usr/include/kddockwidgets-qt6 ] && \
		sudo ln -s $SYSROOT/include/kddockwidgets-qt6 $SYSROOT/usr/include/kddockwidgets-qt6
	popd
}

build_ecm() {
	echo "### Building extra-cmake-modules (ECM) - branch $ECM_BRANCH"
	pushd $STAGING_AREA/extra-cmake-modules
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DBUILD_QTHELP_DOCS=OFF"
	build_with_cmake $1
	popd
}

build_karchive() {
	echo "### Building karchive - version $KARCHIVE_BRANCH"
	pushd $STAGING_AREA/karchive
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr/local"
	build_with_cmake $1
	popd
}

build_genalyzer() {
	echo "### Building genalyzer - branch $GENALYZER_BRANCH"
	pushd $STAGING_AREA/genalyzer
	CURRENT_BUILD_CMAKE_OPTS="\
		-DBUILD_TESTING=OFF \
		-DBUILD_SHARED_LIBS=ON \
		"
	build_with_cmake $1
	popd
}

build_iio-emu() {
	echo "### Building iio-emu - branch $IIOEMU_BRANCH"
	pushd $STAGING_AREA/iio-emu
	build_with_cmake $1
	popd
}

build_scopy() {
	echo "### Building scopy (cross-compile armhf)"
	git config --global --add safe.directory $SRC_DIR
	pushd $SRC_DIR
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_ALL_PACKAGES=ON \
		-DENABLE_PACKAGE_M2K=OFF \
		-DENABLE_PLUGIN_ADC=OFF \
		-DENABLE_PLUGIN_PQM=OFF \
		-DWITH_SIGROK=OFF \
		-DWITH_PYTHON=OFF \
		-DCMAKE_INSTALL_PREFIX=$APP_DIR/usr \
		"
	build_with_cmake OFF
	popd
}

build_deps() {
	clone
	build_libserialport ON
	build_libiio ON
	build_libad9361 ON
	build_libad9166 ON
	build_qwt ON
	build_libtinyiiod ON
	build_kddock ON
	build_ecm ON
	build_karchive ON
	build_genalyzer ON
	build_iio-emu ON
}

create_appdir() {
	BUILD_FOLDER=$SRC_DIR/build
	EMU_BUILD_FOLDER=$STAGING_AREA/iio-emu/build
	COPY_DEPS=$SRC_DIR/ci/arm/copy-deps.sh

	rm -rf $APP_DIR

	pushd ${BUILD_FOLDER}
	make install
	popd

	mkdir -p $APP_DIR/usr/bin
	mkdir -p $APP_DIR/usr/lib
	mkdir -p $APP_DIR/usr/share/applications
	mkdir -p $APP_DIR/usr/share/icons/hicolor/512x512

	cp $APP_RUN $APP_DIR
	cp $APP_DESKTOP $APP_DIR
	cp $SRC_DIR/gui/res/scopy.png $APP_DIR
	cp $SRC_DIR/gui/res/scopy.png $APP_DIR/usr/share/icons/hicolor/512x512
	cp $APP_DESKTOP $APP_DIR/usr/share/applications

	# $EMU_BUILD_FOLDER is $STAGING_AREA/iio-emu/build, i.e. under /home/runner/scripts, which
	# Dockerfile.armhf-cross deletes when it cleans the image - so at CI time this source is
	# guaranteed absent and the old single-source `if` silently shipped no emulator at all.
	# The cross-compiled binary survives at $SYSROOT/bin/iio-emu: build_iio-emu installs with
	# CMAKE_INSTALL_PREFIX=$SYSROOT, which is outside the deleted tree. Scopy then finds it next
	# to its own executable (EmuWidget::findEmuPath -> applicationDirPath, core/src/emuwidget.cpp).
	# Deliberately NOT arm64's `command -v iio-emu` / /usr/local/bin/iio-emu fallbacks: this is a
	# cross build on an x86_64 host, so both of those resolve to x86_64 binaries that would look
	# like a fix and not be executable on the target.
	if [ -f $EMU_BUILD_FOLDER/iio-emu ]; then
		cp $EMU_BUILD_FOLDER/iio-emu $APP_DIR/usr/bin
	elif [ -f $SYSROOT/bin/iio-emu ]; then
		cp $SYSROOT/bin/iio-emu $APP_DIR/usr/bin
	else
		echo "ERROR: no armhf iio-emu in $EMU_BUILD_FOLDER or $SYSROOT/bin."
		echo "       Nothing outside this container can produce an armhf iio-emu, and the AppImage"
		echo "       is the only vehicle that carries it to the device, so refusing to package"
		echo "       without it. Check that build_deps ran build_iio-emu."
		exit 1
	fi

	# Remove system Qt libs that interfere with bundled ones
	sudo rm -rfv ${SYSROOT}/usr/lib/arm-linux-gnueabihf/libQt6* 2>/dev/null || true

	LIB_DIRS=${SYSROOT}:${BUILD_FOLDER}:${QT}/lib
	$COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib $APP_DIR/usr/bin/scopy
	# iio-emu is guaranteed present by the check in create_appdir above, so a "not found" from
	# copy-deps here is a genuine resolution fault rather than a tolerable absence - do not guard it.
	$COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib $APP_DIR/usr/bin/iio-emu
	find $APP_DIR/usr -type f -name 'libscopy*.so' | xargs $COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib

	cp -r $QT/plugins $APP_DIR/usr

	# Copy the Python runtime. With WITH_PYTHON=OFF, PYTHON_VERSION is never written to the cache
	# (core/CMakeLists.txt sets it inside the if(WITH_PYTHON) block), so the grep matches nothing.
	# The trailing `|| true` is required: this script runs under `set -e` and the final `grep -o`
	# exits 1 on no match, which the assignment inherits and which aborted the whole script here -
	# before the guard below could run, making it dead code. Guard on non-empty as well as -d: an
	# empty version would make python_path "$SYSROOT/usr/lib/" and copy the entire sysroot libdir.
	FOUND_PYTHON_VERSION=$(grep 'PYTHON_VERSION' $SRC_DIR/build/CMakeCache.txt | awk -F= '{print $2}' | grep -o 'python[0-9]\+\.[0-9]\+' || true)
	python_path=${SYSROOT}/usr/lib/$FOUND_PYTHON_VERSION
	if [ -n "$FOUND_PYTHON_VERSION" ] && [ -d "$python_path" ]; then
		cp -r $python_path $APP_DIR/usr/lib
	else
		echo "Python runtime not bundled (built with WITH_PYTHON=OFF)"
	fi

	# Copy the protocol decoders. Absent when built with WITH_SIGROK=OFF, which is the case on the
	# slim dependency image - not an error there, so say so instead of finishing silently.
	decoders_found=""
	for search_dir in $SYSROOT/share $SYSROOT/usr/share $SYSROOT/usr/local/share; do
		if [ -d $search_dir/libsigrokdecode/decoders ]; then
			cp -r $search_dir/libsigrokdecode/decoders $APP_DIR/usr/lib
			decoders_found=$search_dir
			break
		fi
	done
	if [ -z "$decoders_found" ]; then
		echo "No decoders for libsigrokdecode found (built with WITH_SIGROK=OFF)"
	fi

	for search_dir in $SYSROOT/lib $SYSROOT/usr/lib $SYSROOT/usr/local/lib; do
		ls $search_dir/libgenalyzer.so* >/dev/null 2>&1 && cp $search_dir/libgenalyzer.so* $APP_DIR/usr/lib && break
	done

	# These are the only Qt libraries nothing scans for: copy-deps.sh runs over scopy and
	# libscopy*.so, never over $QT/plugins, so the platform plugins' own dependencies reach the
	# AppDir only through these copies. A silent miss surfaces at runtime as "could not load the
	# Qt platform plugin", so name each one. All five are present in
	# qt6-armhf-cross-installed.tar.gz, so a WARNING here is a real regression - non-fatal for now,
	# matching arm64, where the same loop printed no warnings and every library landed.
	for qtlib in libQt6XcbQpa libQt6EglFSDeviceIntegration libQt6DBus libQt6OpenGL libQt6WaylandClient; do
		if ls $QT/lib/$qtlib.so* >/dev/null 2>&1; then
			cp $QT/lib/$qtlib.so* $APP_DIR/usr/lib
		else
			echo "WARNING: $qtlib not found in $QT/lib - Qt plugins needing it will fail to load"
		fi
	done

	# Sysroot libraries the same scan misses, each named with the directory it actually ships in.
	for entry in \
		$SYSROOT/lib/$TOOLCHAIN_HOST:libGLESv2 \
		$SYSROOT/lib/$TOOLCHAIN_HOST:libbsd \
		$SYSROOT/usr/lib/$TOOLCHAIN_HOST:libXdmcp \
		$SYSROOT/usr/lib/$TOOLCHAIN_HOST:libXau \
		$SYSROOT/usr/lib/$TOOLCHAIN_HOST:libffi; do
		libdir=${entry%:*}
		libname=${entry##*:}
		if ls $libdir/$libname.so* >/dev/null 2>&1; then
			cp $libdir/$libname.so* $APP_DIR/usr/lib
		else
			echo "WARNING: $libname not found in $libdir"
		fi
	done
}

create_appimage() {
	rm -rf $APP_IMAGE
	mksquashfs $APP_DIR $APP_SQUASHFS -root-owned -noappend
	cat $RUNTIME_ARM >> $APP_IMAGE
	cat $APP_SQUASHFS >> $APP_IMAGE
	chmod a+x $APP_IMAGE
}

move_appimage() {
	mv $APP_IMAGE $SRC_DIR/Scopy-armhf.AppImage
}

generate_appimage() {
	build_scopy
	create_appdir
	create_appimage
	move_appimage
}

configure_system() {
	install_packages
	build_deps
}

for arg in $@; do
	$arg
done

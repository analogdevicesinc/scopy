#!/bin/bash -xe

# Native ARM Build Process for Scopy (Qt6) -- runs on Kuiper Linux (Debian Trixie 13)
# Supports both ARM64 (aarch64) and ARM32 (armhf) -- auto-detected from uname -m
# Usage: ./arm_native_build_process.sh [function_name ...]

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null) || \
SRC_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && cd ../../ && pwd)

SRC_SCRIPT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

MACHINE_ARCH=$(uname -m)
if [ "$MACHINE_ARCH" == "aarch64" ]; then
	ARCH_SUFFIX=arm64
	ARCHITECTURE=aarch64
	TOOLCHAIN_HOST=aarch64-linux-gnu
elif [ "$MACHINE_ARCH" == "armv7l" ] || [ "$MACHINE_ARCH" == "armhf" ]; then
	ARCH_SUFFIX=armhf
	ARCHITECTURE=armhf
	TOOLCHAIN_HOST=arm-linux-gnueabihf
else
	echo "Unsupported architecture: $MACHINE_ARCH"
	exit 1
fi

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

STAGING_AREA=$SRC_SCRIPT/staging
QT_INSTALL_PREFIX=/opt/Qt/6.8.3/gcc_${ARCH_SUFFIX}
QT=$QT_INSTALL_PREFIX
QMAKE_BIN=$QT/bin/qmake6
JOBS=-j$(nproc)

PYTHON_VERSION=$(python3 -c 'import sys; print(f"python3.{sys.version_info.minor}")' 2>/dev/null || echo "python3")

if [ "$MACHINE_ARCH" == "aarch64" ]; then
	CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-aarch64.tar.gz
else
	CMAKE_DOWNLOAD_LINK=""
fi
CMAKE_BIN=$STAGING_AREA/cmake/bin/cmake

APP_DIR=$SRC_SCRIPT/scopy.AppDir
APP_IMAGE=$SRC_SCRIPT/Scopy.AppImage
APP_RUN=$SRC_SCRIPT/../general/AppRun
APP_DESKTOP=$SRC_SCRIPT/../general/scopy.desktop
APP_SQUASHFS=$SRC_SCRIPT/scopy.squashfs
RUNTIME_ARM=$SRC_SCRIPT/runtime-$ARCHITECTURE

QT_DOWNLOAD_LINK=https://download.qt.io/archive/qt/6.8/6.8.3/single/qt-everywhere-src-6.8.3.tar.xz

STAGING_AREA_DEPS=/usr/local
export LD_LIBRARY_PATH=$QT/lib:$LD_LIBRARY_PATH:
# Defines __appimage__ (top-level CMakeLists.txt), which makes the style, translation and package
# paths resolve relative to the executable instead of the build-time install prefix. Without it the
# packaged app cannot find its own theme. Mirrors ci/arm/arm_cross_build_process.sh and
# ci/x86_64/x86-64_appimage_process.sh.
export APPIMAGE=1

CMAKE_OPTS=(\
	-DCMAKE_PREFIX_PATH=$QT \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_VERBOSE_MAKEFILE=ON \
	-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
)

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

download_cmake() {
	echo "#######DOWNLOAD CMAKE#######"
	if [ -z "$CMAKE_DOWNLOAD_LINK" ]; then
		echo "No prebuilt cmake for $MACHINE_ARCH -- using system cmake"
		CMAKE_BIN=$(command -v cmake)
		CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"
		return
	fi
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
	# Use ftp.debian.org -- deb.debian.org CDN can serve stale packages for Trixie
	sudo sed -i 's|http://deb.debian.org|http://ftp.debian.org|g' /etc/apt/sources.list 2>/dev/null || true
	sudo apt-get clean
	sudo apt-get update
	sudo apt-get -y upgrade

	# Remove pre-installed Qt and ADI packages -- will be rebuilt from source
	sudo apt-get -y remove '*qt*' || true
	sudo apt-get -y autoremove || true
	sudo rm -rfv $(find / -path /proc -prune -o -name '*libiio*' -print) 2>/dev/null || true
	sudo rm -rfv $(find / -path /proc -prune -o -name '*libm2k*' -print) 2>/dev/null || true
	sudo rm -rfv $(find / -path /proc -prune -o -name '*libad9361*' -print) 2>/dev/null || true
	sudo rm -rfv $(find / -path /proc -prune -o -name '*libad9166*' -print) 2>/dev/null || true

	# Build tools and libraries for Debian Trixie 13 (ARM64)
	# libfontconfig1-dev/libfreetype-dev/libharfbuzz-dev make Qt6 configure with fontconfig, so
	# arm64 matches armhf (inside_chroot_armhf_qt6.sh:59-60) instead of falling back to the FreeType
	# font DB, which scans <libdir>/fonts and warns because no image ships that directory.
	sudo apt-get -y --no-install-recommends install \
		python3-pip python3-dev python3-numpy python3-packaging python3-mako \
		vim git wget unzip \
		g++ build-essential cmake curl autogen autoconf autoconf-archive pkg-config flex bison \
		mesa-common-dev graphviz gettext texinfo doxygen ninja-build \
		libfftw3-dev \
		libxcb1-dev '^libxcb.*-dev' libxcb-xinerama0 libxcb-icccm4 libxcb-keysyms1 \
		libxcb-shape0-dev libxcb-xkb1 libxcb-cursor-dev libxcb-keysyms1-dev \
		libxcb-icccm4-dev libxcb-image0-dev libxcb-render-util0-dev libxcb-sync-dev \
		libxcb-randr0-dev libxcb-shm0-dev libxcb-util-dev libxcb-xkb-dev libxcb-ewmh-dev \
		libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrender-dev \
		libzip-dev \
		libgl1-mesa-dev libgl-dev libegl-dev libgles-dev \
		libavahi-client-dev libavahi-common-dev \
		libusb-1.0-0 libusb-1.0-0-dev libsndfile1-dev \
		libxkbcommon-x11-0 libncurses-dev libtool libaio-dev libxml2-dev \
		libglu1-mesa-dev libvulkan-dev \
		libzstd-dev libbz2-dev liblzma-dev \
		libxcb-cursor0 libxcb-image0 \
		libdrm-dev libgbm-dev libgles2-mesa-dev \
		libinput-dev libudev-dev \
		libssl-dev libjpeg-dev libpng-dev libxcomposite-dev libdouble-conversion-dev \
		libfontconfig1-dev libfreetype-dev libharfbuzz-dev \
		libwayland-dev libwayland-egl-backend-dev \
		libgsl-dev python-is-python3 \
		libclang-dev libfuse2 squashfs-tools patchelf
}

download_qt6_source() {
	echo "#######DOWNLOAD QT6 SOURCE#######"
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d qt-everywhere-src ]; then
		wget --progress=dot:giga ${QT_DOWNLOAD_LINK}
		tar -xf qt-everywhere-*.tar.xz && rm qt-everywhere-*.tar.xz && mv qt-everywhere-* qt-everywhere-src
	else
		echo "Qt6 source already downloaded"
	fi
	popd
}

configure_qt6() {
	echo "#######CONFIGURE QT6 (NATIVE ARM64)#######"
	mkdir -p ${STAGING_AREA}/build-qt6 && cd ${STAGING_AREA}/build-qt6

	../qt-everywhere-src/configure \
		-release \
		-opensource \
		-confirm-license \
		-prefix "$QT_INSTALL_PREFIX" \
		-nomake examples \
		-nomake tests \
		-opengl desktop \
		-egl \
		-eglfs \
		-xcb \
		-reduce-exports \
		-skip qtandroidextras \
		-skip qtcharts \
		-skip qtdatavis3d \
		-skip qtdoc \
		-skip qtgamepad \
		-skip qtlocation \
		-skip qtlottie \
		-skip qtnetworkauth \
		-skip qtquick3d \
		-skip qtgraphs \
		-skip qtquick3dphysics \
		-skip qtpositioning \
		-skip qtquickcontrols \
		-skip qtquicktimeline \
		-skip qtremoteobjects \
		-skip qtsensors \
		-skip qtspeech \
		-skip qttranslations \
		-skip qtvirtualkeyboard \
		-skip qtwebchannel \
		-skip qtwebengine \
		-skip qtwebsockets \
		-skip qtwebview
}

compile_qt6() {
	echo "#######COMPILE QT6 (NATIVE ARM64)#######"
	cd ${STAGING_AREA}/build-qt6
	cmake --build . --parallel ${QT_BUILD_JOBS:-4}
}

install_qt6() {
	echo "#######INSTALL QT6#######"
	cd ${STAGING_AREA}/build-qt6
	sudo cmake --install .
}

build_qt6() {
	download_cmake
	download_qt6_source
	configure_qt6
	compile_qt6
	install_qt6
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
	pushd $STAGING_AREA/libserialport
	git clean -xdf
	./autogen.sh
	./configure
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
		-DHAVE_DNS_SD:BOOL=OFF\
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
	$QMAKE_BIN qwt.pro
	make $JOBS
	[ "$1" == "ON" ] && sudo make install
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
	# FRONTENDS=qtwidgets drops the Qt Quick/QML backend (Scopy is 100% QtWidgets).
	CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON -DKDDockWidgets_FRONTENDS=qtwidgets"
	build_with_cmake $1
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
	export CMAKE_PREFIX_PATH=$STAGING_AREA_DEPS/share/ECM/cmake:$CMAKE_PREFIX_PATH
	pushd $STAGING_AREA/karchive
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_TESTING=OFF"
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
	echo "### Building scopy"
	git config --global --add safe.directory $SRC_DIR
	ls -la $SRC_DIR
	pushd $SRC_DIR
	# Dependency-rework pass: disable the only gnuradio consumers (adc, pqm) and
	# the sigrok/python core path so the dropped deps are not required.
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_ALL_PACKAGES=ON
		-DENABLE_PACKAGE_M2K=OFF
		-DENABLE_PLUGIN_ADC=OFF
		-DENABLE_PLUGIN_PQM=OFF
		-DWITH_SIGROK=OFF
		-DWITH_PYTHON=OFF
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
	sudo make install
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

	cp /usr/local/bin/scopy $APP_DIR/usr/bin/
	cp /usr/local/lib/libscopy*.so $APP_DIR/usr/lib/
	# The plugin tree, the style and the translations all install under /usr/local/lib/scopy, and with
	# __appimage__ defined the app looks for all three relative to the executable, at
	# usr/lib/scopy/{packages,style,translations} (common/src/scopyconfig.cpp:38,50,161). Silencing a
	# failure here produces an AppImage that launches with no plugins and no theme and says nothing
	# about why, so fail loudly instead.
	mkdir -p $APP_DIR/usr/lib/scopy
	if [ ! -d /usr/local/lib/scopy ] || [ -z "$(ls -A /usr/local/lib/scopy 2>/dev/null)" ]; then
		echo "ERROR: /usr/local/lib/scopy is missing or empty - 'make install' did not install the"
		echo "       plugin tree, style or translations. Refusing to package an AppImage without them."
		exit 1
	fi
	cp -r /usr/local/lib/scopy/* $APP_DIR/usr/lib/scopy/

	if [ -f $EMU_BUILD_FOLDER/iio-emu ]; then
		cp $EMU_BUILD_FOLDER/iio-emu $APP_DIR/usr/bin
	elif command -v iio-emu &>/dev/null; then
		cp $(command -v iio-emu) $APP_DIR/usr/bin
	elif [ -f /usr/local/bin/iio-emu ]; then
		cp /usr/local/bin/iio-emu $APP_DIR/usr/bin
	fi

	LIB_DIRS=/usr/local/lib:/usr/lib/$TOOLCHAIN_HOST:${BUILD_FOLDER}:${QT}/lib
	$COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib $APP_DIR/usr/bin/scopy
	[ -f $APP_DIR/usr/bin/iio-emu ] && $COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib $APP_DIR/usr/bin/iio-emu || true
	$COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib "$(find $APP_DIR/usr -type f -name 'libscopy*.so')"

	# Qt loads the platform plugins with dlopen, so nothing scanned above reaches them: libqxcb
	# alone needs libQt6XcbQpa, which in turn needs libxcb-cursor and ~14 more libxcb siblings.
	# Scan every platform plugin the AppDir ships so this gap cannot recur for the others.
	ls $QT/plugins/platforms/*.so | xargs $COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib

	cp -r $QT/plugins $APP_DIR/usr

	# Copy Python runtime. With WITH_PYTHON=OFF, PYTHON_VERSION is never written to the cache
	# (core/CMakeLists.txt sets it inside the if(WITH_PYTHON) block), so the grep comes back empty.
	# Guard it: an empty version would make python_path "/usr/lib/" and copy all of /usr/lib.
	# The trailing `|| true` is required: this script runs under `set -e`, and the final `grep -o`
	# exits 1 when it matches nothing, which the assignment inherits and which would abort the
	# whole script here - before the check below could run.
	FOUND_PYTHON_VERSION=$(grep 'PYTHON_VERSION' $SRC_DIR/build/CMakeCache.txt | awk -F= '{print $2}' | grep -o 'python[0-9]\+\.[0-9]\+' || true)
	if [ -n "$FOUND_PYTHON_VERSION" ] && [ -d /usr/lib/$FOUND_PYTHON_VERSION ]; then
		cp -r /usr/lib/$FOUND_PYTHON_VERSION $APP_DIR/usr/lib
	else
		echo "Python runtime not bundled (built with WITH_PYTHON=OFF)"
	fi

	# Copy protocol decoders. Absent when built with WITH_SIGROK=OFF, which is the case on the slim
	# dependency images - not an error there, so say so instead of failing silently.
	if [ -d /usr/local/share/libsigrokdecode/decoders ]; then
		cp -r /usr/local/share/libsigrokdecode/decoders $APP_DIR/usr/lib
	else
		echo "No decoders for libsigrokdecode found (built with WITH_SIGROK=OFF)"
	fi

	cp /usr/local/lib/libgenalyzer.so* $APP_DIR/usr/lib 2>/dev/null || true

	# These are the only Qt libraries nothing scans for: copy-deps.sh is run over scopy and
	# libscopy*.so, never over $QT/plugins, so the platform plugins' own dependencies reach the
	# AppDir only through these copies. A silent miss here surfaces at runtime as "could not load
	# the Qt platform plugin", so warn by name. Not fatal: which of these exist depends on how
	# Qt6 was configured on the build host - QtDBus needs libdbus-1-dev, which install_packages
	# does not install, and QT_QPA_PLATFORM in ci/general/AppRun falls back from wayland to xcb.
	for qtlib in libQt6XcbQpa libQt6EglFSDeviceIntegration libQt6DBus libQt6OpenGL libQt6WaylandClient; do
		if ls $QT/lib/$qtlib.so* >/dev/null 2>&1; then
			cp $QT/lib/$qtlib.so* $APP_DIR/usr/lib
		else
			echo "WARNING: $qtlib not found in $QT/lib - Qt plugins needing it will fail to load"
		fi
	done

	cp /usr/lib/$TOOLCHAIN_HOST/libGLESv2.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp /usr/lib/$TOOLCHAIN_HOST/libbsd.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp /usr/lib/$TOOLCHAIN_HOST/libXdmcp.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp /usr/lib/$TOOLCHAIN_HOST/libXau.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp /usr/lib/$TOOLCHAIN_HOST/libffi.so* $APP_DIR/usr/lib 2>/dev/null || true
}

create_appimage() {
	rm -rf $APP_IMAGE
	mksquashfs $APP_DIR $APP_SQUASHFS -root-owned -noappend
	cat $RUNTIME_ARM >> $APP_IMAGE
	cat $APP_SQUASHFS >> $APP_IMAGE
	chmod a+x $APP_IMAGE
}

move_appimage() {
	if [ "$ARCH_SUFFIX" == "arm64" ]; then
		mv $APP_IMAGE $SRC_DIR/Scopy-arm64.AppImage
	elif [ "$ARCH_SUFFIX" == "armhf" ]; then
		mv $APP_IMAGE $SRC_DIR/Scopy-armhf.AppImage
	fi
}

generate_appimage() {
	build_scopy
	create_appdir
	create_appimage
	move_appimage
}

configure_system() {
	install_packages
	build_qt6
	build_deps
}

for arg in $@; do
	$arg
done

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
KDDOCK_BRANCH=2.2
ECM_BRANCH=v6.8.0
KARCHIVE_BRANCH=v6.8.0
GENALYZER_BRANCH=main

STAGING_AREA=$SRC_SCRIPT/staging
SYSROOT=/opt/sysroot/armhf
QT_INSTALL_PREFIX=/opt/Qt/6.8.3/armhf
QT=$QT_INSTALL_PREFIX
JOBS=-j$(nproc)

PYTHON_VERSION=$(python3 -c 'import sys; print(f"python3.{sys.version_info.minor}")' 2>/dev/null || echo "python3")

CMAKE_DOWNLOAD_LINK=https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz
CMAKE_BIN=$STAGING_AREA/cmake/bin/cmake

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
	sudo apt-get update
	sudo apt-get -y upgrade
	sudo apt-get -y install \
		build-essential cmake git wget ca-certificates unzip \
		g++ pkg-config flex bison autogen autoconf autoconf-archive libtool swig \
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
		libdouble-conversion-dev libglib2.0-dev \
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
	[ -d 'libm2k' ]		|| git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
	[ -d 'spdlog' ]		|| git clone --recursive https://github.com/gabime/spdlog.git -b $SPDLOG_BRANCH spdlog
	[ -d 'gr-scopy' ]	|| git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
	[ -d 'gr-m2k' ]		|| git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
	[ -d 'volk' ]		|| git clone --recursive https://github.com/gnuradio/volk.git -b $VOLK_BRANCH volk
	[ -d 'gnuradio' ]	|| git clone --recursive https://github.com/analogdevicesinc/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'libsigrokdecode' ] || git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
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

build_libm2k() {
	echo "### Building libm2k - branch $LIBM2K_BRANCH"
	pushd $STAGING_AREA/libm2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DENABLE_CSHARP=OFF \
		-DBUILD_EXAMPLES=OFF \
		-DENABLE_TOOLS=OFF \
		-DINSTALL_UDEV_RULES=OFF \
		"
	build_with_cmake $1
	popd
}

build_spdlog() {
	echo "### Building spdlog - branch $SPDLOG_BRANCH"
	pushd $STAGING_AREA/spdlog
	CURRENT_BUILD_CMAKE_OPTS="-DSPDLOG_BUILD_SHARED=ON"
	build_with_cmake $1
	popd
}

build_volk() {
	echo "### Building volk - branch $VOLK_BRANCH"
	pushd $STAGING_AREA/volk
	CURRENT_BUILD_CMAKE_OPTS="-DPYTHON_EXECUTABLE=/usr/bin/python3 -DENABLE_ORC=OFF"
	build_with_cmake $1
	popd
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"
	pushd $STAGING_AREA/gnuradio
	CURRENT_BUILD_CMAKE_OPTS="\
		-DPYTHON_EXECUTABLE=/usr/bin/python3 \
		-DENABLE_DEFAULT=OFF \
		-DENABLE_GNURADIO_RUNTIME=ON \
		-DENABLE_GR_ANALOG=ON \
		-DENABLE_GR_BLOCKS=ON \
		-DENABLE_GR_FFT=ON \
		-DENABLE_GR_FILTER=ON \
		-DENABLE_GR_IIO=ON \
		-DENABLE_POSTINSTALL=OFF
		"
	build_with_cmake $1
	popd
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"
	pushd $STAGING_AREA/gr-m2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DDIGITAL=OFF
		"
	build_with_cmake $1
	popd
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
	pushd $STAGING_AREA/gr-scopy
	build_with_cmake $1
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	set_config_opts
	pushd $STAGING_AREA/libsigrokdecode
	git clean -xdf
	./autogen.sh
	./configure "${CONFIG_OPTS[@]}"
	make $JOBS
	[ "$1" == "ON" ] && sudo make install
	popd
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	pushd $STAGING_AREA/qwt
	git clean -xdf
	sed -i 's|/usr/local/qwt-$$QWT_VERSION-ma|/usr/local|g' qwtconfig.pri
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
	CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON"
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
	build_spdlog ON
	build_libm2k ON
	build_volk ON
	build_gnuradio ON
	build_grscopy ON
	build_grm2k ON
	build_qwt ON
	build_libsigrokdecode ON
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

	if [ -f $EMU_BUILD_FOLDER/iio-emu ]; then
		cp $EMU_BUILD_FOLDER/iio-emu $APP_DIR/usr/bin
	fi

	# Remove system Qt libs that interfere with bundled ones
	sudo rm -rfv ${SYSROOT}/usr/lib/arm-linux-gnueabihf/libQt6* 2>/dev/null || true

	LIB_DIRS=${SYSROOT}:${BUILD_FOLDER}:${QT}/lib
	$COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib $APP_DIR/usr/bin/scopy
	[ -f $APP_DIR/usr/bin/iio-emu ] && $COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib $APP_DIR/usr/bin/iio-emu || true
	find $APP_DIR/usr -type f -name 'libscopy*.so' | xargs $COPY_DEPS --lib-dir ${LIB_DIRS} --output-dir $APP_DIR/usr/lib

	cp -r $QT/plugins $APP_DIR/usr

	FOUND_PYTHON_VERSION=$(grep 'PYTHON_VERSION' $SRC_DIR/build/CMakeCache.txt | awk -F= '{print $2}' | grep -o 'python[0-9]\+\.[0-9]\+')
	python_path=${SYSROOT}/usr/lib/$FOUND_PYTHON_VERSION
	[ -d $python_path ] && cp -r $python_path $APP_DIR/usr/lib || true

	for search_dir in $SYSROOT/share $SYSROOT/usr/share $SYSROOT/usr/local/share; do
		[ -d $search_dir/libsigrokdecode/decoders ] && cp -r $search_dir/libsigrokdecode/decoders $APP_DIR/usr/lib && break
	done

	for search_dir in $SYSROOT/lib $SYSROOT/usr/lib $SYSROOT/usr/local/lib; do
		ls $search_dir/libgenalyzer.so* >/dev/null 2>&1 && cp $search_dir/libgenalyzer.so* $APP_DIR/usr/lib && break
	done

	cp $QT/lib/libQt6XcbQpa.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6EglFSDeviceIntegration.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6DBus.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6OpenGL.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6WaylandClient.so* $APP_DIR/usr/lib 2>/dev/null || true

	cp $SYSROOT/lib/$TOOLCHAIN_HOST/libGLESv2.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $SYSROOT/lib/$TOOLCHAIN_HOST/libbsd.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $SYSROOT/usr/lib/$TOOLCHAIN_HOST/libXdmcp.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $SYSROOT/usr/lib/$TOOLCHAIN_HOST/libXau.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $SYSROOT/usr/lib/$TOOLCHAIN_HOST/libffi.so* $APP_DIR/usr/lib 2>/dev/null || true
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

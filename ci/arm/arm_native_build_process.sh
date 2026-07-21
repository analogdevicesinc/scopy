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
	sudo apt-get -y --no-install-recommends install \
		python3-pip python3-dev python3-numpy python3-packaging python3-mako \
		vim git wget unzip \
		g++ build-essential cmake curl autogen autoconf autoconf-archive pkg-config flex bison swig \
		mesa-common-dev graphviz gettext texinfo doxygen ninja-build \
		libboost-all-dev libfftw3-dev \
		libxcb1-dev '^libxcb.*-dev' libxcb-xinerama0 libxcb-icccm4 libxcb-keysyms1 \
		libxcb-shape0-dev libxcb-xkb1 libxcb-cursor-dev libxcb-keysyms1-dev \
		libxcb-icccm4-dev libxcb-image0-dev libxcb-render-util0-dev libxcb-sync-dev \
		libxcb-randr0-dev libxcb-shm0-dev libxcb-util-dev libxcb-xkb-dev libxcb-ewmh-dev \
		libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrender-dev \
		libgmp3-dev libzip-dev libglib2.0-dev \
		libmatio-dev liborc-0.4-dev libgl1-mesa-dev libgl-dev libegl-dev libgles-dev \
		libavahi-client-dev libavahi-common-dev \
		libusb-1.0-0 libusb-1.0-0-dev libsndfile1-dev \
		libxkbcommon-x11-0 libncurses-dev libtool libaio-dev libzmq3-dev libxml2-dev \
		libglu1-mesa-dev libvulkan-dev \
		libzstd-dev libbz2-dev liblzma-dev \
		libxcb-cursor0 libxcb-image0 \
		libdrm-dev libgbm-dev libgles2-mesa-dev \
		libinput-dev libudev-dev \
		libssl-dev libjpeg-dev libpng-dev libxcomposite-dev libdouble-conversion-dev \
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

build_glog() {
	echo "### Building glog - branch $GLOG_BRANCH"
	pushd $STAGING_AREA/glog
	CURRENT_BUILD_CMAKE_OPTS="-DWITH_GFLAGS=OFF"
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
	CURRENT_BUILD_CMAKE_OPTS="-DPYTHON_EXECUTABLE=/usr/bin/python3"
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
	pushd $STAGING_AREA/libsigrokdecode
	git clean -xdf
	./autogen.sh
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	./configure
	make $JOBS
	[ "$INSTALL" == "ON" ] && sudo make install
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
	CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON"
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
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_ALL_PACKAGES=ON
		-DENABLE_PACKAGE_M2K=OFF
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
	mkdir -p $APP_DIR/usr/lib/scopy
	cp -r /usr/local/lib/scopy/* $APP_DIR/usr/lib/scopy/ 2>/dev/null || true

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

	cp -r $QT/plugins $APP_DIR/usr

	FOUND_PYTHON_VERSION=$(grep 'PYTHON_VERSION' $SRC_DIR/build/CMakeCache.txt | awk -F= '{print $2}' | grep -o 'python[0-9]\+\.[0-9]\+')
	python_path=/usr/lib/$FOUND_PYTHON_VERSION
	[ -d $python_path ] && cp -r $python_path $APP_DIR/usr/lib || true

	cp -r /usr/local/share/libsigrokdecode/decoders $APP_DIR/usr/lib 2>/dev/null || true
	cp /usr/local/lib/libgenalyzer.so* $APP_DIR/usr/lib 2>/dev/null || true

	cp $QT/lib/libQt6XcbQpa.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6EglFSDeviceIntegration.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6DBus.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6OpenGL.so* $APP_DIR/usr/lib 2>/dev/null || true
	cp $QT/lib/libQt6WaylandClient.so* $APP_DIR/usr/lib 2>/dev/null || true

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

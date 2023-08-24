#!/bin/bash

set -ex

REPO_SRC=$BUILD_REPOSITORY_LOCALPATH

LIBIIO_VERSION=v0.25
LIBAD9361_BRANCH=master
LIBM2K_BRANCH=master
GNURADIO_BRANCH=maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=master
QWT_BRANCH=qwt-multiaxes
LIBSIGROKDECODE_BRANCH=master
LIBTINYIIOD_BRANCH=master
GLOG_BRANCH=v0.4.0
SPDLOG_BRANCH=v1.x

PYTHON="python3"
QT_FORMULAE=qt@5

PACKAGES=" ${QT_FORMULAE} volk spdlog boost pkg-config cmake fftw bison gettext autoconf automake libtool libzip glib libusb glog $PYTHON"
PACKAGES="$PACKAGES doxygen wget gnu-sed libmatio dylibbundler libxml2 ghr libserialport libsndfile"

STAGING_AREA=$PWD/staging
JOBS=-j8
STAGINGDIR=$STAGING_AREA/dependencies

source ${REPO_SRC}/ci/macOS/before_install_lib.sh

install_packages() {
	brew search ${QT_FORMULAE}
	brew install $PACKAGES
	for pkg in gcc bison gettext cmake python; do
		brew link --overwrite --force $pkg
	done

	pip3 install mako
}

export_paths(){
	QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
	export PATH="/usr/local/bin:$PATH"
	export PATH="/usr/local/opt/bison/bin:$PATH"
	export PATH="${QT_PATH}:$PATH"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libzip/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$STAGINGDIR/lib/pkgconfig"

	QMAKE="$(command -v qmake)"
	CMAKE_BIN="$(command -v cmake)"
	CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGINGDIR -DCMAKE_INSTALL_PREFIX=$STAGINGDIR"
	CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

	echo -- USING CMAKE COMMAND:
	echo $CMAKE
	echo -- USING QT: $QT_PATH
	echo -- USING QMAKE: $QMAKE_BIN
	echo -- PATH: $PATH
	echo -- PKG_CONFIG_PATH: $PKG_CONFIG_PATH
}

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
	git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
	git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
	git clone --recursive https://github.com/gnuradio/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
	git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	popd
}

generate_status_file(){
	# Generate build status info for the about page
	BUILD_STATUS_FILE=${REPO_SRC}/build-status
	brew list --versions $PACKAGES > $BUILD_STATUS_FILE
}

save_version_info() {
	echo "$CURRENT_BUILD - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
}

build_with_cmake() {
	echo $PWD
	BUILD_FOLDER=$PWD/build
	rm -rf $BUILD_FOLDER
	git clean -xdf
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $CURRENT_BUILD_CMAKE_OPTS ../
	make $JOBS
	sudo make $JOBS install

	#clear variable
	CURRENT_BUILD_CMAKE_OPTS=""
}

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"
	CURRENT_BUILD=libiio
	save_version_info

	pushd $STAGING_AREA/libiio
	CURRENT_BUILD_CMAKE_OPTS="\
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_DOC:BOOL=OFF \
		-DHAVE_DNS_SD:BOOL=ON \
		-DENABLE_DNS_SD:BOOL=ON \
		-DWITH_MATLAB_BINDINGS:BOOL=OFF \
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		-DINSTALL_UDEV_RULE:BOOL=OFF \
		-DWITH_SERIAL_BACKEND:BOOL=ON \
		-DENABLE_IPV6:BOOL=OFF \
		-DOSX_PACKAGE:BOOL=OFF
		"
	build_with_cmake
	popd
}

build_libm2k() {
	echo "### Building libm2k - branch $LIBM2K_BRANCH"
	pushd $STAGING_AREA/libm2k
	CURRENT_BUILD=libm2k
	save_version_info

	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DENABLE_CSHARP=OFF \
		-DBUILD_EXAMPLES=OFF \
		-DENABLE_TOOLS=OFF \
		-DINSTALL_UDEV_RULES=OFF \
		-DENABLE_LOG=OFF\
		"
	build_with_cmake
	popd
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	CURRENT_BUILD=libad9361-iio
	save_version_info

	pushd $STAGING_AREA/libad9361
	build_with_cmake
	popd
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"
	CURRENT_BUILD=gnuradio
	save_version_info

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
	build_with_cmake
	popd
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"
	CURRENT_BUILD=gr-m2k
	save_version_info

	pushd $STAGING_AREA/gr-m2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DDIGITAL=OFF
		"
	build_with_cmake
	popd
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
	CURRENT_BUILD=gr-scopy
	save_version_info

	pushd $STAGING_AREA/gr-scopy
	CURRENT_BUILD_CMAKE_OPTS="-DWITH_PYTHON=OFF "
	build_with_cmake
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	CURRENT_BUILD=libsigrokdecode
	save_version_info

	pushd $STAGING_AREA/libsigrokdecode
	git reset --hard
	./autogen.sh
	./configure --prefix $STAGINGDIR
	sudo make $JOBS install
	popd
}

build_qwt() {
	echo "### Building qwt - branch qwt-multiaxes"
	CURRENT_BUILD=qwt
	save_version_info

	qmake_build_local "qwt" "qwt.pro" "patch_qwt"
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	CURRENT_BUILD=libtinyiiod
	save_version_info

	pushd $STAGING_AREA/libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	build_with_cmake
	popd
}

build_deps(){
	build_libiio
	build_libad9361
	build_libm2k
	build_gnuradio
	build_grscopy
	build_grm2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
}

install_packages
export_paths
clone
generate_status_file
build_deps

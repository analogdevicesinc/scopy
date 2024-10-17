#!/bin/bash -xe

## Set STAGING
USE_STAGING=OFF
##

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

[ "$CI_SCRIPT" == "ON" ] && USE_STAGING=OFF

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

QT=/opt/Qt/5.15.2/gcc_64
QMAKE_BIN=$QT/bin/qmake
CMAKE_BIN=/bin/cmake
JOBS=-j14
STAGING_AREA=$SRC_SCRIPT/staging_ubuntu20

if [ "$USE_STAGING" == "ON" ]
	then
		echo -- USING STAGING FOLDER: $STAGING_AREA_DEPS
		STAGING_AREA_DEPS=$STAGING_AREA/dependencies
		export LD_LIBRARY_PATH=$STAGING_AREA_DEPS/lib:$QT/lib:$LD_LIBRARY_PATH
		CMAKE_OPTS=(\
			-DCMAKE_LIBRARY_PATH=$STAGING_AREA_DEPS \
			-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
			-DCMAKE_PREFIX_PATH=$QT\;$STAGING_AREA_DEPS \
			-DCMAKE_EXE_LINKER_FLAGS=-L$STAGING_AREA_DEPS\;-L$STAGING_AREA_DEPS/lib \
			-DCMAKE_SHARED_LINKER_FLAGS=-L$STAGING_AREA_DEPS\;-L$STAGING_AREA_DEPS/lib \
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
		echo  -- STAGING_DIR $STAGING_AREA_DEPS
	else
		echo -- NO STAGING: INSTALLING IN SYSTEM
		STAGING_AREA_DEPS=/usr/local
		export LD_LIBRARY_PATH=$QT/lib:$LD_LIBRARY_PATH:
		CMAKE_OPTS=(\
			-DCMAKE_PREFIX_PATH=$QT \
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
fi

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"
echo -- USING CMAKE COMMAND:
echo $CMAKE
echo -- USING QT: $QT
echo -- USING QMAKE: $QMAKE_BIN

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
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
	popd
}

build_with_cmake() {
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	BUILD_FOLDER=$PWD/build
	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $CURRENT_BUILD_CMAKE_OPTS ../
	make $JOBS
	if [ "$INSTALL" == "ON" ]; then
		if [ "$USE_STAGING" == "ON" ]; then make install; else sudo make install; fi
	fi
	CURRENT_BUILD_CMAKE_OPTS=""
}

install_packages() {
	sudo apt update
	sudo apt -y upgrade
	sudo apt -y --no-install-recommends install autoconf autogen bison build-essential cmake curl doxygen flex g++ git graphviz libaio-dev libavahi-client-dev \
		libboost-all-dev libfftw3-3 libfftw3-bin libfftw3-dev libgmp3-dev libglib2.0-dev libglibmm-2.4-dev \
		libgl1-mesa-dev liblog4cpp5-dev liblog4cpp5v5 libmatio-dev liborc-0.4-dev libpython3-all-dev libserialport0 \
		libserialport-dev libsigc++-2.0-dev libsndfile1-dev libtool libusb-1.0-0-dev libxml2-dev libzip-dev \
		libzmq3-dev mesa-common-dev pkg-config python3-dev python3 python3-numpy python3-pip subversion swig vim wget unzip

	pip3 install --no-cache-dir mako
	pip3 install --no-cache-dir packaging
}

install_qt() {
	# installing Qt using the aqt tool https://github.com/miurahr/aqtinstall
	sudo pip3 install --no-cache-dir aqtinstall
	sudo python3 -m aqt install-qt --outputdir /opt/Qt linux desktop 5.15.2
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
	INSTALL="ON"
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

	if [ "$USE_STAGING" == "ON" ]; then
		./configure --prefix $STAGING_AREA_DEPS
		LD_RUN_PATH=$STAGING_AREA_DEPS/lib make $JOBS
	else
		./configure
		make $JOBS
	fi

	if [ "$INSTALL" == "ON" ];then
		if [ "$USE_STAGING" == "ON" ]; then make install; else sudo make install; fi
	fi
	popd
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	pushd $STAGING_AREA/qwt
	git clean -xdf
	sed -i 's|/usr/local/qwt-$$QWT_VERSION-ma|/usr/local|g' qwtconfig.pri
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON

	if [ "$USE_STAGING" == "ON" ]; then
		$QMAKE_BIN INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
		make $JOBS
		if [ "$INSTALL" == "ON" ];then
			make INSTALL_ROOT=$STAGING_AREA_DEPS install
		fi
		cp -r $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
	else
		$QMAKE_BIN qwt.pro
		make $JOBS
		if [ "$INSTALL" == "ON" ];then
			sudo make install
		fi
	fi
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	pushd $STAGING_AREA/libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
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
	ls -la $SRC_DIR
	pushd $SRC_DIR
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PLUGIN_TEST=ON \
		-DENABLE_TESTING=ON
		"
	build_with_cmake OFF
	popd
}

#
# Helper functions
#

build_deps(){
	clone
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
	build_iio-emu ON
}

configure_system(){
	install_packages
	install_qt
	build_deps
}

for arg in $@; do
	$arg
done

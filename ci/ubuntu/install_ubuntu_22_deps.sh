#!/bin/bash
set -xe

USE_STAGING=$1

LIBIIO_VERSION=master
LIBAD9361_BRANCH=master
GLOG_BRANCH=v0.4.0

LIBM2K_BRANCH=master
SPDLOG_BRANCH=v1.x
VOLK_BRANCH=main
GNURADIO_BRANCH=maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=master
LIBSIGROKDECODE_BRANCH=master
QWT_BRANCH=qwt-multiaxes
LIBTINYIIOD_BRANCH=master

SRC_DIR=$GITHUB_WORKSPACE
STAGING_AREA=$PWD/staging
STAGING_AREA_DEPS=$STAGING_AREA/dependencies
QT=/home/runner/5.15.2/gcc_64 # this is used to force the use of Qt5.15 for qt_add_resources
QMAKE_BIN=$QT/bin/qmake
CMAKE_BIN=/bin/cmake
JOBS=-j8
ARCH=x86_64

if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		echo -- USING STAGING
		mkdir -p $STAGING_AREA_DEPS
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STAGING_AREA_DEPS/lib:$QT/lib
		CMAKE_OPTS=(\
			-DCMAKE_LIBRARY_PATH=$STAGING_AREA_DEPS \
			-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
			-DCMAKE_PREFIX_PATH=$QT\;$STAGING_AREA_DEPS \
			-DCMAKE_EXE_LINKER_FLAGS="-L$STAGING_AREA_DEPS -L$STAGING_AREA_DEPS/lib" \
			-DCMAKE_SHARED_LINKER_FLAGS="-L$STAGING_AREA_DEPS -L$STAGING_AREA_DEPS/lib" \
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
		echo  -- STAGING_DIR $STAGING_AREA_DEPS
	else
		echo -- NO STAGING
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$QT/lib
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
	git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	git clone --recursive https://github.com/google/glog.git -b $GLOG_BRANCH glog
	git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
	git clone --recursive https://github.com/gabime/spdlog.git -b $SPDLOG_BRANCH spdlog
	git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
	git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
	git clone --recursive https://github.com/gnuradio/volk.git -b $VOLK_BRANCH volk
	git clone --recursive https://github.com/gnuradio/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
	git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	popd
}


build_with_cmake() {
	echo $PWD
	BUILD_FOLDER=$PWD/build-${ARCH}
	rm -rf $BUILD_FOLDER
	git clean -xdf
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $CURRENT_BUILD_CMAKE_OPTS ../
	make $JOBS
	sudo make $JOBS install
	sudo ldconfig
}

update(){
	sudo apt-get update
	sudo apt-get -y upgrade
}

install_apt() {
	sudo DEBIAN_FRONTEND=noninteractive apt-get -y install keyboard-configuration
	sudo apt-get -y install vim git cmake libgmp3-dev libboost-all-dev libxml2-dev libxml2 flex bison swig \
	libpython3-all-dev python3 python3-pip python3-numpy libfftw3-bin libfftw3-dev libfftw3-3 liblog4cpp5v5 \
	liblog4cpp5-dev g++ autoconf libzip-dev libglib2.0-dev libsigc++-2.0-dev libglibmm-2.4-dev \
	libclang-dev doxygen curl libmatio-dev liborc-0.4-dev subversion mesa-common-dev libgl1-mesa-dev libserialport0 \
	libserialport-dev libusb-1.0 libusb-1.0-0 libusb-1.0-0-dev libtool libaio-dev libzmq3-dev libsndfile1-dev \
	libavahi-client-dev graphviz build-essential
	pip3 install mako
	pip3 install packaging
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
	build_with_cmake
	popd
}

build_glog() {
	echo "### Building glog - branch $GLOG_BRANCH"
	pushd $STAGING_AREA/glog
	CURRENT_BUILD_CMAKE_OPTS="-DWITH_GFLAGS=OFF"
	build_with_cmake
	popd
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	pushd $STAGING_AREA/libad9361
	build_with_cmake
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
		-DENABLE_LOG=ON
		"
	build_with_cmake
	popd
}

build_spdlog() {
	echo "### Building spdlog - branch $SPDLOG_BRANCH"
	pushd $STAGING_AREA/spdlog
	CURRENT_BUILD_CMAKE_OPTS="-DSPDLOG_BUILD_SHARED=ON"
	build_with_cmake
	popd
}

build_volk() {
	echo "### Building volk - branch $VOLK_BRANCH"
	pushd $STAGING_AREA/volk
	CURRENT_BUILD_CMAKE_OPTS="-DPYTHON_EXECUTABLE=/usr/bin/python3"
	build_with_cmake \
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
	build_with_cmake	
	popd
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"
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
	pushd $STAGING_AREA/gr-scopy
	build_with_cmake
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	pushd $STAGING_AREA/libsigrokdecode
	./autogen.sh
	if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		./configure --prefix $STAGING_AREA_DEPS
	else
		./configure``
	fi
	sudo make $JOBS install
	sudo ldconfig
	popd
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	pushd $STAGING_AREA/qwt

	if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		$QMAKE_BIN INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
		make $JOBS
		sudo make INSTALL_ROOT=$STAGING_AREA_DEPS install
		sudo cp -r $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
	else
		$QMAKE_BIN qwt.pro
		make $JOBS
		sudo make install
	fi

	sudo ldconfig
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	pushd $STAGING_AREA/libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	build_with_cmake
	popd
}

build_scopy() {
	echo "### Building scopy"
	ls -la $SRC_DIR
	pushd $SRC_DIR
	rm -rf $SRC_DIR/build-$ARCH
	mkdir -p $SRC_DIR/build-$ARCH
	cd $SRC_DIR/build-$ARCH
	$CMAKE \
		-DCMAKE_LIBRARY_PATH=$STAGING_AREA_DEPS \
		-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
		-DCMAKE_PREFIX_PATH=$STAGING_AREA_DEPS\;$QT \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DENABLE_PLUGIN_TEST=ON \
		-DENABLE_TESTING=ON \
		../
	make $JOBS
	popd
}

build_deps(){
	build_libiio
	build_libad9361
	build_glog
	build_spdlog
	build_libm2k
	build_volk
	build_gnuradio
	build_grscopy
	build_grm2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
}

#clone
#update
#install_apt
#install_qt
#build_deps
#build_scopy

#!/bin/bash
set -ex
export PS4='+(${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

JOBS=-j1
SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || echo "No source directory found"
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

STAGING_AREA=$SRC_SCRIPT/staging
STAGING_AREA_DEPS=$STAGING_AREA/dependencies


USE_STAGING=OFF

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
KDDOCK_BRANCH=2.1
ECM_BRANCH=kf5
KARCHIVE_BRANCH=kf5

if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		echo -- USING STAGING
		mkdir -p $STAGING_AREA_DEPS
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STAGING_AREA_DEPS/lib
		CMAKE_OPTS=(\
			-DCMAKE_LIBRARY_PATH=$STAGING_AREA_DEPS \
			-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
			-DCMAKE_PREFIX_PATH=$STAGING_AREA_DEPS \
			-DCMAKE_EXE_LINKER_FLAGS="-L$STAGING_AREA_DEPS -L$STAGING_AREA_DEPS/lib" \
			-DCMAKE_SHARED_LINKER_FLAGS="-L$STAGING_AREA_DEPS -L$STAGING_AREA_DEPS/lib" \
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
		echo  -- STAGING_DIR $STAGING_AREA_DEPS
	else
		echo -- NO STAGING
		mkdir -p $STAGING_AREA
		CMAKE_OPTS=(\
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
fi

CMAKE_BIN=/bin/cmake
CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"
echo -- USING CMAKE COMMAND:
echo $CMAKE

build_with_cmake() {
	echo $PWD
	BUILD_FOLDER=$PWD/build
	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE "$@"
	make $JOBS
	sudo make $JOBS install
}

clone() {
	echo "####### CLONE #######"
	pushd $STAGING_AREA
	[ -d 'libserialport' ] || git clone --recursive https://github.com/sigrokproject/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	[ -d 'libiio' ]		|| git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	[ -d 'libad9361' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	[ -d 'libm2k' ]		|| git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
	[ -d 'spdlog' ]		|| git clone --recursive https://github.com/gabime/spdlog.git -b $SPDLOG_BRANCH spdlog
	[ -d 'volk' ]		|| git clone --recursive https://github.com/gnuradio/volk.git -b $VOLK_BRANCH volk
	[ -d 'gnuradio' ]	|| git clone --recursive https://github.com/analogdevicesinc/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	[ -d 'gr-scopy' ]	|| git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
	[ -d 'gr-m2k' ]		|| git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'libsigrokdecode' ] || git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
	[ -d 'libtinyiiod' ]	|| git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	[ -d 'iio-emu' ]	|| git clone --recursive https://github.com/analogdevicesinc/iio-emu -b $IIOEMU_BRANCH iio-emu
	[ -d 'KDDockWidgets' ] || git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	[ -d 'extra-cmake-modules' ] || git clone --recursive https://github.com/KDE/extra-cmake-modules.git -b $ECM_BRANCH extra-cmake-modules
	[ -d 'karchive' ] || git clone --recursive https://github.com/KDE/karchive.git -b $KARCHIVE_BRANCH karchive
	popd
}

install_apt() {
	sudo apt-get update
	sudo apt-get -y upgrade
	sudo apt-get -y install build-essential cmake vim bison flex swig swig4.0 python3 mlocate libfftw3-dev libgsl-dev \
		libusb-1.0-* libavahi-client* libavahi-common* libxml2* libsndfile-dev libfuse2 libboost1.74-* libglib2.0-dev \
		qtbase5-dev* qt5-qmake* qttools5-dev* qtdeclarative5-dev libqt5qml* libqt5svg5* libgmp3-dev libgmp-dev libthrift-dev libunwind-dev
	pip install mako --break-system-packages
}

build_libserialport(){
	echo "####### BUILD LIBSERIALPORT #######"
	pushd $STAGING_AREA/libserialport
	git clean -xdf
	./autogen.sh
	./configure ${AUTOCONF_OPTS}
	make $JOBS
	sudo make install
	popd
}

build_libiio() {
	echo "####### BUILD LIBIIO #######"
	pushd $STAGING_AREA/libiio
	build_with_cmake \
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_DOC:BOOL=OFF \
		-DHAVE_DNS_SD:BOOL=ON\
		-DWITH_MATLAB_BINDINGS:BOOL=OFF \
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		-DWITH_SERIAL_BACKEND:BOOL=ON \
		-DENABLE_IPV6:BOOL=OFF \
		-DINSTALL_UDEV_RULE:BOOL=OFF\
		../
	popd
}

build_libad9361() {
	echo "####### BUILD LIBAD9361 #######"
	pushd $STAGING_AREA/libad9361
	build_with_cmake
	popd
}

build_libm2k() {
	echo "####### BUILD LIBM2K #######"
	pushd $STAGING_AREA/libm2k
	build_with_cmake -DENABLE_PYTHON=OFF -DENABLE_TOOLS=ON ../
	popd
}

build_spdlog() {
	echo "####### BUILD SPDLOG #######"
	pushd $STAGING_AREA/spdlog
	build_with_cmake -DSPDLOG_BUILD_SHARED=ON ../
	popd
}

build_volk() {
	echo "####### BUILD VOLK #######"
	pushd $STAGING_AREA/volk
	build_with_cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DPYTHON_EXECUTABLE=/usr/bin/python3 ../
	popd
}

build_gnuradio() {
	echo "####### BUILD GNURADIO #######"
	pushd $STAGING_AREA/gnuradio
	build_with_cmake \
		-DPYTHON_EXECUTABLE=/usr/bin/python3 \
		-DENABLE_DEFAULT=OFF \
		-DENABLE_GNURADIO_RUNTIME=ON \
		-DENABLE_GR_ANALOG=ON \
		-DENABLE_GR_BLOCKS=ON \
		-DENABLE_GR_FFT=ON \
		-DENABLE_GR_FILTER=ON \
		-DENABLE_GR_IIO=ON \
		-DENABLE_POSTINSTALL=OFF \
		../
	popd
}

build_gr_scopy() {
	echo "####### BUILD GR_SCOPY #######"
	pushd $STAGING_AREA/gr-scopy
	build_with_cmake -DWITH_PYTHON=OFF  ../
	popd
}

build_gr_m2k() {
	echo "####### BUILD GR_M2K #######"
	pushd $STAGING_AREA/gr-m2k
	build_with_cmake -DWITH_PYTHON=OFF  -DDIGITAL=OFF ../
	popd
}

build_qwt() {
	echo "####### BUILD QWT #######"
	pushd $STAGING_AREA/qwt
	git clean -xdf
	sed -i 's|/usr/local/qwt-$$QWT_VERSION-ma|/usr/local|g' qwtconfig.pri
	if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		qmake INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
		make $JOBS
		make INSTALL_ROOT=$STAGING_AREA_DEPS install
		cp -r $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
	else
		qmake qwt.pro
		make $JOBS
		sudo make install
	fi

	popd
}

build_libsigrokdecode() {
	echo "####### BUILD LIBSIGROKDECODE #######"
	pushd $STAGING_AREA/libsigrokdecode
	git clean -xdf
	./autogen.sh

	if [ ! -z "$USE_STAGING" ] && [ "$USE_STAGING" == "ON" ]
	then
		./configure --prefix $STAGING_AREA_DEPS
	else
		./configure
	fi

	make $JOBS
	sudo make install
	popd
}

build_libtinyiiod() {
	echo "####### BUILD LIBTINYIIOD #######"
	pushd $STAGING_AREA/libtinyiiod
	build_with_cmake -DBUILD_EXAMPLES=OFF ../
	popd
}

build_kddock () {
	echo "### Building KDDockWidgets - version $KDDOCK_BRANCH"
	pushd $STAGING_AREA/KDDockWidgets
	build_with_cmake ../
	sudo make install
	popd
}

build_ecm() {
	echo "### Building extra-cmake-modules (ECM) - branch $ECM_BRANCH"
	pushd $STAGING_AREA/extra-cmake-modules
	build_with_cmake ../
	popd
}

build_karchive () {
	echo "### Building karchive - version $KARCHIVE_BRANCH"
    pushd $STAGING_AREA/karchive
    build_with_cmake ../
    popd
}

build_iio-emu() {
	echo "####### BUILD IIO-EMU #######"
	pushd $STAGING_AREA/iio-emu
	build_with_cmake ../
	popd
}

build_scopy() {
	echo "####### BUILD SCOPY #######"
	pushd $SRC_DIR
	build_with_cmake ../
	popd
}

build_scopy_appimage()
{
	pushd $SRC_DIR
	export APPIMAGE=1
	build_with_cmake ../
	popd

	$SRC_DIR/ci/arm/arm_build_process.sh create_appdir create_appimage
}

buid_deps() {
	build_libserialport
	build_libiio
	build_libad9361
	build_libm2k
	build_spdlog
	build_volk
	build_gnuradio
	build_gr_scopy
	build_gr_m2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
	build_kddock
	build_ecm
	build_karchive
}


# install_apt
# clone
# buid_deps
# build_scopy

for arg in $@; do
	$arg
done


#!/bin/bash
set -ex
export PS4='+(${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

STAGING_AREA=$HOME"/staging"
STAGING_AREA_DEPS=$STAGING_AREA"/dependencies"
JOBS=-j1

USE_STAGING=OFF

LIBIIO_VERSION=v0.25
LIBM2K_BRANCH=main
SPDLOG_BRANCH=v1.x
VOLK_BRANCH=main
GNURADIO_BRANCH=scopy2-maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=master
LIBSIGROKDECODE_BRANCH=master
QWT_BRANCH=qwt-multiaxes-updated
LIBTINYIIOD_BRANCH=master
IIOEMU_BRANCH=master
SCOPY_BRANCH=dev

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
	sudo ldconfig
}

clone() {
	echo "#######CLONE#######"
	pushd $STAGING_AREA
	[ -d 'libiio' ]		|| git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
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
	[ -d 'scopy' ]		|| git clone --recursive https://github.com/analogdevicesinc/scopy -b $SCOPY_BRANCH scopy
	popd
}

install_apt() {
	sudo apt-get update
	sudo apt-get -y upgrade
	sudo apt-get -y install build-essential cmake vim bison flex swig swig4.0 python3 mlocate \
		libusb-1.0-* libavahi-client* libavahi-common* libxml2* libsndfile-dev libfuse2 libboost1.74-* \
		qtbase5-dev* qt5-qmake* qttools5-dev* qtdeclarative5-dev libqt5qml* libqt5svg5*
	pip install mako --break-system-packages
}

build_libiio() {
	echo "#######build_libiio#######"
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

build_libm2k() {
	echo "#######build_libm2k#######"
	pushd $STAGING_AREA/libm2k
	build_with_cmake -DENABLE_PYTHON=OFF -DENABLE_TOOLS=ON ../
	popd
}

build_spdlog() {
	echo "#######build_spdlog#######"
	pushd $STAGING_AREA/spdlog
	build_with_cmake -DSPDLOG_BUILD_SHARED=ON ../
	popd
}

build_volk() {
	echo "#######build_volk#######"
	pushd $STAGING_AREA/volk
	build_with_cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DPYTHON_EXECUTABLE=/usr/bin/python3 ../
	popd
}

build_gnuradio() {
	echo "#######build_gnuradio#######"
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
	echo "#######build_gr_scopy#######"
	pushd $STAGING_AREA/gr-scopy
	build_with_cmake -DWITH_PYTHON=OFF  ../
	popd
}

build_gr_m2k() {
	echo "#######build_gr_m2k#######"
	pushd $STAGING_AREA/gr-m2k
	build_with_cmake -DWITH_PYTHON=OFF  -DDIGITAL=OFF ../
	popd
}

build_qwt() {
	echo "#######build_qwt#######"
	pushd $STAGING_AREA/qwt
	git clean -xdf
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

	sudo ldconfig
	popd
}

build_libsigrokdecode() {
	echo "#######build_libsigrokdecode#######"
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
	sudo ldconfig
	popd
}

build_libtinyiiod() {
	echo "#######build_libtinyiiod#######"
	pushd $STAGING_AREA/libtinyiiod
	build_with_cmake -DBUILD_EXAMPLES=OFF ../
	popd
}

build_iio-emu() {
	echo "#######build_iio-emu#######"
	pushd $STAGING_AREA/iio-emu
	build_with_cmake ../
	popd
}

build_scopy() {
	echo "#######build_scopy#######"
	pushd $STAGING_AREA/scopy
	build_with_cmake ../
	popd
}

test_scopy() {
	echo "#######TEST_SCOPY#######"
	pushd $STAGING_AREA/scopy/build
	./scopy
	popd
}

buid_deps() {
	build_libiio
	build_libm2k
	build_spdlog
	build_volk
	build_gnuradio
	build_gr_scopy
	build_gr_m2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
}


# instal_apt
# clone
# buid_deps
# build_scopy
# test_scopy

for arg in $@; do
	$arg
done


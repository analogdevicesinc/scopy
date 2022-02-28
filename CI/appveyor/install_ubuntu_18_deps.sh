#!/bin/bash

LIBIIO_VERSION=0ed18cd8f6b2fac5204a99e38922bea73f1f778c
LIBAD9361_BRANCH=master
GLOG_BRANCH=v0.4.0
LIBM2K_BRANCH=master
GRIIO_BRANCH=upgrade-3.8
GNURADIO_FORK=analogdevicesinc
GNURADIO_BRANCH=scopy
GRSCOPY_BRANCH=master
GRM2K_BRANCH=master
QWT_BRANCH=qwt-multiaxes
LIBSIGROK_BRANCH=master
LIBSIGROKDECODE_BRANCH=master
LIBTINYIIOD_BRANCH=master

set -e
if [ $# -eq 0 ]; then
	echo "Using default qmake"
	QMAKE=qmake
	$QMAKE --version
else
	QMAKE=$1/gcc_64/bin/qmake
	$QMAKE --version

fi

cd ~
WORKDIR=${PWD}

install_apt() {
	sudo add-apt-repository -y ppa:gnuradio/gnuradio-releases
	sudo apt-get update

	curl -fsSL -o doxygen.tar.gz "https://phoenixnap.dl.sourceforge.net/project/doxygen/rel-1.8.17/doxygen-1.8.17.linux.bin.tar.gz"
	tar -xzf doxygen.tar.gz
	sudo cp -a doxygen-1.8.17/bin/doxy* /usr/local/bin
	doxygen --version

	sudo apt-get -y install build-essential libxml2-dev libxml2 flex bison swig libpython3-all-dev python3 python3-numpy libfftw3-bin libfftw3-dev libfftw3-3 liblog4cpp5v5 liblog4cpp5-dev libboost1.65-dev libboost1.65 g++ git cmake autoconf libzip4 libzip-dev libglib2.0-dev libsigc++-2.0-dev libglibmm-2.4-dev curl libvolk1-bin libvolk1-dev libvolk1.3 libgmp-dev libmatio-dev liborc-0.4-dev subversion mesa-common-dev libgl1-mesa-dev libserialport0 libserialport-dev libusb-1.0 libusb-1.0-0 libusb-1.0-0-dev libaio-dev

	sudo apt-get -y update
	sudo apt-get -y install gnuradio
}

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"

	cd ~
	git clone https://github.com/analogdevicesinc/libiio.git ${WORKDIR}/libiio
	cd ${WORKDIR}/libiio
	git checkout $LIBIIO_VERSION

	mkdir ${WORKDIR}/libiio/build-${ARCH}
	cd ${WORKDIR}/libiio/build-${ARCH}
	# Download a 32-bit version of windres.exe

	cmake ${CMAKE_OPTS} \
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_DOC:BOOL=OFF \
		-DHAVE_DNS_SD:BOOL=OFF\
		-DWITH_MATLAB_BINDINGS:BOOL=OFF \
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		${WORKDIR}/libiio

	make $JOBS
	sudo make ${JOBS} install
#	DESTDIR=${WORKDIR} make ${JOBS} install
}

build_glog() {

	echo "### Building glog - branch $GLOG_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/google/glog.git -b $GLOG_BRANCH ${WORKDIR}/glog

	mkdir ${WORKDIR}/glog/build-${ARCH}
	cd ${WORKDIR}/glog/build-${ARCH}

	cmake	${CMAKE_OPTS} \
		-DWITH_GFLAGS=OFF\
		${WORKDIR}/glog

	make $JOBS
	sudo make ${JOBS} install
	#DESTDIR=${WORKDIR} make ${JOBS} install
}

build_libm2k() {

	echo "### Building libm2k - branch $LIBM2K_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH ${WORKDIR}/libm2k

	mkdir ${WORKDIR}/libm2k/build-${ARCH}
	cd ${WORKDIR}/libm2k/build-${ARCH}

	cmake	${CMAKE_OPTS} \
		-DENABLE_PYTHON=OFF\
		-DENABLE_CSHARP=OFF\
		-DENABLE_EXAMPLES=OFF\
		-DENABLE_TOOLS=OFF\
		-DINSTALL_UDEV_RULES=OFF\
		-DENABLE_LOG=ON\
		${WORKDIR}/libm2k

	make $JOBS
	sudo make ${JOBS} install
	#DESTDIR=${WORKDIR} make ${JOBS} install
}
build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH ${WORKDIR}/libad9361

	mkdir ${WORKDIR}/libad9361/build-${ARCH}
	cd ${WORKDIR}/libad9361/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		${WORKDIR}/libad9361

	make $JOBS
	sudo make $JOBS install
	#DESTDIR=${WORKDIR} make $JOBS install
}

build_griio() {
	echo "### Building gr-iio - branch $GRIIO_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/gr-iio.git -b $GRIIO_BRANCH ${WORKDIR}/gr-iio
	mkdir ${WORKDIR}/gr-iio/build-${ARCH}
	cd ${WORKDIR}/gr-iio/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		${WORKDIR}/gr-iio

	make $JOBS
	sudo make $JOBS install
	#DESTDIR=${WORKDIR} make $JOBS install
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH ${WORKDIR}/gr-m2k
	mkdir ${WORKDIR}/gr-m2k/build-${ARCH}
	cd ${WORKDIR}/gr-m2k/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		${WORKDIR}/gr-m2k

	make $JOBS
	sudo make $JOBS install
	#DESTDIR=${WORKDIR} make $JOBS install

}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH ${WORKDIR}/gr-scopy
	mkdir ${WORKDIR}/gr-scopy/build-${ARCH}
	cd ${WORKDIR}/gr-scopy/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		${WORKDIR}/gr-scopy

	make $JOBS
	sudo make $JOBS install
	#DESTDIR=${WORKDIR} make $JOBS install
}

build_libsigrok() {
	echo "### Building libsigrok - branch $LIBSIGROK_BRANCH"

	git clone --depth 1 https://github.com/sigrokproject/libsigrok.git -b $LIBSIGROK_BRANCH ${WORKDIR}/libsigrok

	mkdir ${WORKDIR}/libsigrok/build-${ARCH}
	cd ${WORKDIR}/libsigrok

	./autogen.sh
	./configure --disable-all-drivers --enable-bindings --enable-cxx

	sudo make $JOBS install
	#DESTDIR=${WORKDIR} make $JOBS install

	# For some reason, Scopy chokes if these are present in enums.hpp
	#sed -i "s/static const Quantity \* const DIFFERENCE;$//g" ${WORKDIR}/msys64/${MINGW_VERSION}/include/libsigrokcxx/enums.hpp
	#sed -i "s/static const QuantityFlag \* const RELATIVE;$//g" ${WORKDIR}/msys64/${MINGW_VERSION}/include/libsigrokcxx/enums.hpp
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"

	git clone --depth 1 https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH ${WORKDIR}/libsigrokdecode

	cd ${WORKDIR}/libsigrokdecode

	./autogen.sh
	./configure

	sudo make $JOBS install
	#DESTDIR=${WORKDIR} make $JOBS install
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"

	git clone https://github.com/cseci/qwt --branch $QWT_BRANCH ${WORKDIR}/qwt
	cd ${WORKDIR}/qwt

	$QMAKE qwt.pro
	make $JOBS
	sudo make install
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH ${WORKDIR}/libtinyiiod
	mkdir ${WORKDIR}/libtinyiiod/build-${ARCH}
	cd ${WORKDIR}/libtinyiiod/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DBUILD_EXAMPLES=OFF \
		${WORKDIR}/libtinyiiod

	make $JOBS
	sudo make $JOBS install
}

install_apt
build_libiio
build_libad9361
build_glog
build_libm2k
build_griio
build_grscopy
build_grm2k
build_qwt
build_libsigrokdecode
build_libtinyiiod

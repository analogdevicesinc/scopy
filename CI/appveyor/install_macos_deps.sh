#!/bin/bash

LIBIIO_VERSION=v0.24-rc.1
LIBAD9361_BRANCH=master
LIBM2K_BRANCH=master
GRIIO_BRANCH=upgrade-3.8
GNURADIO_FORK=analogdevicesinc
GNURADIO_BRANCH=scopy
GRSCOPY_BRANCH=master
GRM2K_BRANCH=master
QWT_BRANCH=qwt-multiaxes
LIBSIGROKDECODE_BRANCH=master
LIBTINYIIOD_BRANCH=master

PYTHON="python3"
PACKAGES=" ${QT_FORMULAE} boost pkg-config cmake fftw bison gettext autoconf automake libtool libzip glib libusb glog $PYTHON"
PACKAGES="$PACKAGES doxygen wget gnu-sed libmatio dylibbundler libxml2 ghr"

set -e
REPO_SRC=$BUILD_REPOSITORY_LOCALPATH
WORKDIR=${PWD}
JOBS=4

brew search ${QT_FORMULAE}
brew install $PACKAGES

for pkg in gcc bison gettext cmake python; do
	brew link --overwrite --force $pkg
done

pip3 install mako six

pwd
source ${REPO_SRC}/CI/appveyor/before_install_lib.sh

QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"

export PATH="/usr/local/bin:$PATH"
export PATH="/usr/local/opt/bison/bin:$PATH"
export PATH="${QT_PATH}:$PATH"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libzip/lib/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$STAGINGDIR/lib/pkgconfig"

echo $PKG_CONFIG_PATH
QMAKE="$(command -v qmake)"

CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGINGDIR -DCMAKE_INSTALL_PREFIX=$STAGINGDIR"

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"

	git clone https://github.com/analogdevicesinc/libiio.git ${WORKDIR}/libiio
	cd ${WORKDIR}/libiio
	git checkout $LIBIIO_VERSION

	mkdir ${WORKDIR}/libiio/build-${ARCH}
	cd ${WORKDIR}/libiio/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_DOC:BOOL=OFF \
		-DWITH_MATLAB_BINDINGS:BOOL=OFF \
		-DENABLE_DNS_SD:BOOL=OFF\
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		-DOSX_PACKAGE:BOOL=OFF \
		${WORKDIR}/libiio

	make -j $JOBS
	sudo make -j ${JOBS} install
}

build_libm2k() {

	echo "### Building libm2k - branch $LIBM2K_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH ${WORKDIR}/libm2k

	mkdir ${WORKDIR}/libm2k/build-${ARCH}
	cd ${WORKDIR}/libm2k/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DENABLE_PYTHON=OFF \
		-DENABLE_CSHARP=OFF \
		-DBUILD_EXAMPLES=OFF \
		-DENABLE_TOOLS=OFF \
		-DINSTALL_UDEV_RULES=OFF \
		-DENABLE_LOG=ON\
		${WORKDIR}/libm2k

	make -j $JOBS
	sudo make -j ${JOBS} install
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH ${WORKDIR}/libad9361

	mkdir ${WORKDIR}/libad9361/build-${ARCH}
	cd ${WORKDIR}/libad9361/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		${WORKDIR}/libad9361

	make -j $JOBS
	sudo make -j $JOBS install
}

build_log4cpp() {
	cd ${WORKDIR}
	wget https://sourceforge.net/projects/log4cpp/files/latest/log4cpp-1.1.3.tar.gz
	tar xvzf log4cpp-1.1.3.tar.gz
	cd log4cpp
	./configure --prefix=/usr/local/
	make -j $JOBS
	sudo make -j ${JOBS} install
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"

	git clone --recurse-submodules https://github.com/$GNURADIO_FORK/gnuradio -b $GNURADIO_BRANCH ${WORKDIR}/gnuradio
	mkdir ${WORKDIR}/gnuradio/build-${ARCH}
	cd ${WORKDIR}/gnuradio/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DENABLE_GR_DIGITAL:BOOL=OFF \
		-DENABLE_GR_DTV:BOOL=OFF \
		-DENABLE_GR_AUDIO:BOOL=OFF \
		-DENABLE_GR_CHANNELS:BOOL=OFF \
		-DENABLE_GR_TRELLIS:BOOL=OFF \
		-DENABLE_GR_VOCODER:BOOL=OFF \
		-DENABLE_GR_QTGUI:BOOL=OFF \
		-DENABLE_GR_FEC:BOOL=OFF \
		-DENABLE_SPHINX:BOOL=OFF \
		-DENABLE_DOXYGEN:BOOL=OFF \
		-DENABLE_INTERNAL_VOLK=ON \
		-DENABLE_PYTHON=OFF \
		-DENABLE_TESTING=OFF \
		-DENABLE_GR_CHANNELS=OFF \
		-DENABLE_GR_VOCODER=OFF \
		-DENABLE_GR_TRELLIS=OFF \
		-DENABLE_GR_WAVELET=OFF \
		-DENABLE_GR_CTRLPORT=OFF \
		-DENABLE_CTRLPORT_THRIFT=OFF \
		-DCMAKE_C_FLAGS=-fno-asynchronous-unwind-tables \
		${WORKDIR}/gnuradio
	make -j $JOBS
	sudo make -j $JOBS install
}

build_griio() {
	echo "### Building gr-iio - branch $GRIIO_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/gr-iio.git -b $GRIIO_BRANCH ${WORKDIR}/gr-iio
	mkdir ${WORKDIR}/gr-iio/build-${ARCH}
	cd ${WORKDIR}/gr-iio/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DWITH_PYTHON=OFF \
		${WORKDIR}/gr-iio

	make -j $JOBS
	sudo make -j $JOBS install
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH ${WORKDIR}/gr-m2k
	mkdir ${WORKDIR}/gr-m2k/build-${ARCH}
	cd ${WORKDIR}/gr-m2k/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DWITH_PYTHON=OFF \
		${WORKDIR}/gr-m2k

	make -j $JOBS
	sudo make -j $JOBS install
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH ${WORKDIR}/gr-scopy
	mkdir ${WORKDIR}/gr-scopy/build-${ARCH}
	cd ${WORKDIR}/gr-scopy/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DWITH_PYTHON=OFF \
		${WORKDIR}/gr-scopy

	make -j $JOBS
	sudo make -j $JOBS install
}
build_glibmm() {
	echo "### Building glibmm - 2.64.0"
	cd ${WORKDIR}
	wget http://ftp.acc.umu.se/pub/gnome/sources/glibmm/2.64/glibmm-2.64.0.tar.xz
	tar xzvf glibmm-2.64.0.tar.xz
	cd glibmm-2.64.0
	./configure --prefix=$STAGINGDIR
	make -j $JOBS
	sudo make -j $JOBS install
}

build_sigcpp() {
	echo "### Building libsigc++ -2.10.0"
	cd ${WORKDIR}
	wget http://ftp.acc.umu.se/pub/GNOME/sources/libsigc++/2.10/libsigc++-2.10.0.tar.xz
	tar xvzf libsigc++-2.10.0.tar.xz
	cd libsigc++-2.10.0
	./configure --prefix=$STAGINGDIR
	make -j $JOBS
	sudo make -j $JOBS install
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"

	git clone --depth 1 https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH ${WORKDIR}/libsigrokdecode
	cd ${WORKDIR}/libsigrokdecode

	./autogen.sh
	./configure --prefix=$STAGINGDIR

	sudo make -j $JOBS install
}

build_qwt() {
	echo "### Building qwt - branch qwt-multiaxes"
	git clone --depth 1 https://github.com/cseci/qwt -b $QWT_BRANCH ${WORKDIR}/qwt
	qmake_build_local "qwt" "qwt.pro" "patch_qwt"
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH ${WORKDIR}/libtinyiiod
	mkdir ${WORKDIR}/libtinyiiod/build-${ARCH}
	cd ${WORKDIR}/libtinyiiod/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DBUILD_EXAMPLES=OFF \
		${WORKDIR}/libtinyiiod

	make -j $JOBS
	sudo make -j $JOBS install
}

build_sigcpp
build_glibmm
build_libiio
build_libad9361
build_libm2k
build_log4cpp
build_gnuradio
build_griio
build_grscopy
build_grm2k
build_qwt
build_libsigrokdecode
build_libtinyiiod

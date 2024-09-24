#!/bin/bash

set -xe

LIBIIO_VERSION=libiio-v0
LIBAD9361_BRANCH=main
LIBM2K_BRANCH=src/output_triggers
GNURADIO_BRANCH=maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=main
QWT_BRANCH=qwt-multiaxes
LIBSIGROKDECODE_BRANCH=decoders/AD559XR
LIBTINYIIOD_BRANCH=master

PYTHON="python3"
PACKAGES=" ${QT_FORMULAE} volk spdlog boost pkg-config cmake fftw bison gettext autoconf automake libtool libzip glib libusb glog"
PACKAGES="$PACKAGES doxygen wget gnu-sed libmatio dylibbundler libxml2 ghr libsndfile"

REPO_SRC=$BUILD_REPOSITORY_LOCALPATH
WORKDIR=${PWD}
JOBS=4

# Workaround: Homebrew fails to upgrade Python's 2to3 due to conflicting symlinks  https://github.com/actions/runner-images/issues/6817
rm /usr/local/bin/2to3 || true
rm /usr/local/bin/idle3 || true
rm /usr/local/bin/pydoc3 || true
rm /usr/local/bin/python3 || true
rm /usr/local/bin/python3-config || true

brew update
# Workaround for brew taking a long time to upgrade existing packages
# Check if macOS version and upgrade packages only if the version is greater than macOS 12
if (( $(echo "$(sw_vers -productVersion) > 13.0" | bc -l) )); then
	brew upgrade --display-times || true #ignore homebrew upgrade errors
	brew install --display-times $PACKAGES
else
	HOMEBREW_NO_AUTO_UPDATE=1 brew install --display-times $PACKAGES
fi

brew search ${QT_FORMULAE}
brew install --display-times $PACKAGES
for pkg in gcc bison gettext cmake python; do
	brew link --overwrite --force $pkg
done

pip3 install --break-system-packages mako

# Generate build status info for the about page
BUILD_STATUS_FILE=${REPO_SRC}/build-status
brew list --versions $PACKAGES > $BUILD_STATUS_FILE

source ${REPO_SRC}/CI/appveyor/before_install_lib.sh

QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"

export PATH="/usr/local/bin:$PATH"
export PATH="/usr/local/opt/bison/bin:$PATH"
export PATH="${QT_PATH}:$PATH"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libzip/lib/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$STAGINGDIR/lib/pkgconfig"
export PKG_CONFIG_PATH="$(brew --prefix python3)/lib/pkgconfig:$PKG_CONFIG_PATH"
export PATH="$(brew --prefix python3)/bin:$PATH"

QMAKE="$(command -v qmake)"
CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGINGDIR -DCMAKE_INSTALL_PREFIX=$STAGINGDIR"

export 

save_version_info() {
	echo "$CURRENT_BUILD - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
}

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"

	git clone https://github.com/analogdevicesinc/libiio.git ${WORKDIR}/libiio
	cd ${WORKDIR}/libiio
	git checkout $LIBIIO_VERSION

	mkdir ${WORKDIR}/libiio/build-${ARCH}
	cd ${WORKDIR}/libiio/build-${ARCH}
	CURRENT_BUILD=libiio
	save_version_info

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
	CURRENT_BUILD=libm2k
	save_version_info

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
	CURRENT_BUILD=libad9361-iio
	save_version_info

	cmake ${CMAKE_OPTS} \
		${WORKDIR}/libad9361

	make -j $JOBS
	sudo make -j $JOBS install
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"

	git clone --recurse-submodules https://github.com/gnuradio/gnuradio.git -b $GNURADIO_BRANCH ${WORKDIR}/gnuradio
	mkdir ${WORKDIR}/gnuradio/build-${ARCH}
	cd ${WORKDIR}/gnuradio/build-${ARCH}
	CURRENT_BUILD=gnuradio
	save_version_info

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

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH ${WORKDIR}/gr-m2k
	mkdir ${WORKDIR}/gr-m2k/build-${ARCH}
	cd ${WORKDIR}/gr-m2k/build-${ARCH}
	CURRENT_BUILD=gr-m2k
	save_version_info

	cmake ${CMAKE_OPTS} \
		-DENABLE_PYTHON=OFF \
		-DDIGITAL=OFF \
		${WORKDIR}/gr-m2k

	make -j $JOBS
	sudo make -j $JOBS install
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH ${WORKDIR}/gr-scopy
	mkdir ${WORKDIR}/gr-scopy/build-${ARCH}
	cd ${WORKDIR}/gr-scopy/build-${ARCH}
	CURRENT_BUILD=gr-scopy
	save_version_info

	cmake ${CMAKE_OPTS} \
		-DWITH_PYTHON=OFF \
		${WORKDIR}/gr-scopy

	make -j $JOBS
	sudo make -j $JOBS install
}
build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH ${WORKDIR}/libsigrokdecode
	cd ${WORKDIR}/libsigrokdecode
	CURRENT_BUILD=libsigrokdecode
	save_version_info

	./autogen.sh
	./configure --prefix=$STAGINGDIR

	sudo make -j $JOBS install
}

build_qwt() {
	echo "### Building qwt - branch qwt-multiaxes"
	git clone --depth 1 https://github.com/cseci/qwt -b $QWT_BRANCH ${WORKDIR}/qwt
	CURRENT_BUILD=qwt
	save_version_info
	qmake_build_local "qwt" "qwt.pro" "patch_qwt"
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"

	git clone --depth 1 https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH ${WORKDIR}/libtinyiiod
	mkdir ${WORKDIR}/libtinyiiod/build-${ARCH}
	cd ${WORKDIR}/libtinyiiod/build-${ARCH}
	CURRENT_BUILD=libtinyiiod
	save_version_info

	cmake ${CMAKE_OPTS} \
		-DBUILD_EXAMPLES=OFF \
		${WORKDIR}/libtinyiiod

	make -j $JOBS
	sudo make -j $JOBS install
}

build_libiio
build_libad9361
build_libm2k
build_gnuradio
build_grscopy
build_grm2k
build_qwt
build_libsigrokdecode
build_libtinyiiod

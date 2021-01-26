#!/bin/bash

LIBIIO_BRANCH=master
LIBAD9361_BRANCH=master
LIBM2K_BRANCH=master
GRIIO_BRANCH=upgrade-3.8
GNURADIO_FORK=analogdevicesinc
GNURADIO_BRANCH=ming-3.8-clean
GRSCOPY_BRANCH=master
GRM2K_BRANCH=master
QWT_BRANCH=qwt-6.1-multiaxes
QWTPOLAR_BRANCH=master # not used
LIBSIGROK_BRANCH=master
LIBSIGROKDECODE_BRANCH=master #not used
BOOST_VERSION_FILE=1_73_0
BOOST_VERSION=1.73.0

PYTHON="python3"
PACKAGES=" qt pkg-config cmake fftw bison gettext autoconf automake libtool libzip glib libusb $PYTHON"
PACKAGES="$PACKAGES glibmm doxygen wget gnu-sed libmatio dylibbundler libxml2"

set -e
cd ~
WORKDIR=${PWD}
NUM_JOBS=4

brew_install_or_upgrade() {
	brew install $1 || \
		brew upgrade $1 || \
		brew ls --versions $1 # check if installed last-ly
}

brew_install() {
	brew install $1 || \
		brew ls --versions $1
}

for pak in $PACKAGES ; do
	brew_install $pak
done

for pkg in gcc bison gettext cmake python; do
	brew link --overwrite --force $pkg
done

pip3 install mako six

pwd
source ./projects/scopy/CI/appveyor/before_install_lib.sh

QT_PATH="$(brew --prefix qt)/bin"
export PATH="/usr/local/bin:$PATH"
export PATH="/usr/local/opt/bison/bin:$PATH"
export PATH="${QT_PATH}:$PATH"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libzip/lib/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/glibmm/lib/pkgconfig"

QMAKE="$(command -v qmake)"

CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGINGDIR -DCMAKE_INSTALL_PREFIX=$STAGINGDIR"

build_libiio() {
	echo "### Building libiio - branch $LIBIIO_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_BRANCH ${WORKDIR}/libiio

	mkdir ${WORKDIR}/libiio/build-${ARCH}
	cd ${WORKDIR}/libiio/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_DOC:BOOL=OFF \
		-DWITH_MATLAB_BINDINGS:BOOL=OFF \
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		-DOSX_PACKAGE:BOOL=OFF \
		${WORKDIR}/libiio

	make $JOBS
	sudo make ${JOBS} install
}

build_libm2k() {

	echo "### Building libm2k - branch $LIBM2K_BRANCH"

	cd ~
	git clone --depth 1 https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH ${WORKDIR}/libm2k

	mkdir ${WORKDIR}/libm2k/build-${ARCH}
	cd ${WORKDIR}/libm2k/build-${ARCH}

	cmake ${CMAKE_OPTS} \
		-DENABLE_PYTHON=OFF \
		-DENABLE_CSHARP=OFF \
		-DBUILD_EXAMPLES=OFF \
		-DENABLE_TOOLS=OFF \
		-DINSTALL_UDEV_RULES=OFF \
		${WORKDIR}/libm2k

	make $JOBS
	sudo make ${JOBS} install
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

build_log4cpp() {
	wget https://sourceforge.net/projects/log4cpp/files/latest/log4cpp-1.1.3.tar.gz
	tar xvzf log4cpp-1.1.3.tar.gz
	cd log4cpp
	./configure --prefix=/usr/local/
	make $JOBS
	sudo make install
}

build_boost() {
	echo "### Building boost - version $BOOST_VERSION_FILE"

	cd ~
	wget https://dl.bintray.com/boostorg/release/$BOOST_VERSION/source/boost_$BOOST_VERSION_FILE.tar.gz
	tar -xzf boost_$BOOST_VERSION_FILE.tar.gz
	cd boost_$BOOST_VERSION_FILE
	patch -p1 <  ${WORKDIR}/projects/scopy/CI/appveyor/patches/boost-darwin.patch
	./bootstrap.sh --with-libraries=atomic,date_time,filesystem,program_options,system,chrono,thread,regex,test
	./b2
	./b2 install
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"

	cd ~
	git clone --recurse-submodules https://github.com/$GNURADIO_FORK/gnuradio -b $GNURADIO_BRANCH
	mkdir ${WORKDIR}/gnuradio/build-${ARCH}
	cd ${WORKDIR}/gnuradio/build-${ARCH}

	cmake 	-DENABLE_GR_DIGITAL:BOOL=OFF \
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
		-DCMAKE_C_FLAGS=-fno-asynchronous-unwind-tables \
		${WORKDIR}/gnuradio
	make $JOBS
	sudo make install
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
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"

	git clone --depth 1 https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH ${WORKDIR}/libsigrokdecode
	mkdir -p ${WORKDIR}/libsigrokdecode/build-${ARCH}
	cd ${WORKDIR}/libsigrokdecode

	./autogen.sh
	./configure
	sudo make $JOBS install
}


patch_qwtpolar_mac() {
	patch_qwtpolar

	patch -p1 <<-EOF
--- a/qwtpolarconfig.pri
+++ b/qwtpolarconfig.pri
@@ -16,7 +16,9 @@ QWT_POLAR_VER_PAT      = 1
 QWT_POLAR_VERSION      = \$\${QWT_POLAR_VER_MAJ}.\$\${QWT_POLAR_VER_MIN}.\$\${QWT_POLAR_VER_PAT}
 
 unix {
-    QWT_POLAR_INSTALL_PREFIX    = /usr/local/qwtpolar-\$\$QWT_POLAR_VERSION
+    QWT_POLAR_INSTALL_PREFIX    = $STAGINGDIR
+    QMAKE_CXXFLAGS              = -I${STAGINGDIR}/include
+    QMAKE_LFLAGS                = ${STAGINGDIR}/lib/libqwt*dylib
 }
 
 win32 {
EOF
}

build_qwt() {
	echo "### Building qwt - branch qwt-6.1-multiaxes"
	svn checkout https://svn.code.sf.net/p/qwt/code/branches/$QWT_BRANCH ${WORKDIR}/qwt
	qmake_build_local "qwt" "qwt.pro" "patch_qwt"
}

build_qwtpolar() {
	qmake_build_wget "qwtpolar-1.1.1" "https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2" "qwtpolar.pro" "patch_qwtpolar_mac"
}

build_libiio
build_libad9361
build_libm2k
build_log4cpp
build_boost
build_gnuradio
build_griio
build_grscopy
build_grm2k
build_qwt
build_qwtpolar
build_libsigrokdecode






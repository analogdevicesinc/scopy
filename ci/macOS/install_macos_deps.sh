#!/bin/bash

set -ex
REPO_SRC=$(git rev-parse --show-toplevel)
source $REPO_SRC/ci/macOS/macos_config.sh

PACKAGES="${QT_FORMULAE} volk spdlog boost pkg-config cmake fftw bison gettext autoconf automake libzip glib libusb glog "
PACKAGES="$PACKAGES doxygen wget gnu-sed libmatio dylibbundler libxml2 ghr libsndfile"

OS_VERSION=${1:-$(sw_vers -productVersion)}
echo "MacOS version $OS_VERSION"

source ${REPO_SRC}/ci/macOS/before_install_lib.sh

install_packages() {

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
		# Workaround: Install or update libtool package only if macOS version is greater than 12
		# Note: libtool (v2.4.7) is pre-installed by default, but it can be updated to v2.5.3
		PACKAGES="$PACKAGES libtool"
		brew install --display-times $PACKAGES
	else
		HOMEBREW_NO_AUTO_UPDATE=1 brew install --display-times $PACKAGES
	fi

	pip3 install --break-system-packages mako
}

export_paths(){
	QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
	export PATH="/usr/local/bin:$PATH"
	export PATH="/usr/local/opt/bison/bin:$PATH"
	export PATH="${QT_PATH}:$PATH"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libzip/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$STAGING_AREA_DEPS/lib/pkgconfig"

	QMAKE="$(command -v qmake)"
	CMAKE_BIN="$(command -v cmake)"
	CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGING_AREA_DEPS -DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS"
	CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

	echo -- USING CMAKE COMMAND:
	echo $CMAKE
	echo -- USING QT: $QT_PATH
	echo -- USING QMAKE: $QMAKE
	echo -- PATH: $PATH
	echo -- PKG_CONFIG_PATH: $PKG_CONFIG_PATH
}

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	git clone --recursive https://github.com/cseci/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
	git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
	git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
	git clone --recursive https://github.com/analogdevicesinc/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
	git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	popd
}

generate_status_file(){
	# Generate build status info for the about page
	BUILD_STATUS_FILE=${REPO_SRC}/build-status
	brew list --versions $PACKAGES > $BUILD_STATUS_FILE
}

save_version_info() {
	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE
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

	#clear variable
	CURRENT_BUILD_CMAKE_OPTS=""
}

build_libserialport(){
	CURRENT_BUILD=libserialport
	pushd $STAGING_AREA/$CURRENT_BUILD
	save_version_info
	git clean -xdf
	./autogen.sh
	./configure --prefix $STAGING_AREA_DEPS
	make $JOBS
	sudo make install
	popd
}

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"
	CURRENT_BUILD=libiio
	pushd $STAGING_AREA/libiio
	save_version_info
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
	sudo make install
	sudo chmod -R 775 $STAGING_AREA_DEPS
	sudo chmod 664 $STAGING_AREA_DEPS/lib/pkgconfig/libiio.pc
	cp -R $STAGING_AREA/libiio/build/iio.framework $STAGING_AREA_DEPS/lib
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
	make install
	popd
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	CURRENT_BUILD=libad9361-iio
	pushd $STAGING_AREA/libad9361
	save_version_info
	build_with_cmake
	make install
	popd
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"
	CURRENT_BUILD=gnuradio
	pushd $STAGING_AREA/gnuradio
	save_version_info
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
	make install
	popd
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"
	CURRENT_BUILD=gr-m2k
	pushd $STAGING_AREA/gr-m2k
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DDIGITAL=OFF
		"
	build_with_cmake
	make install
	popd
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
	CURRENT_BUILD=gr-scopy
	pushd $STAGING_AREA/gr-scopy
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="-DWITH_PYTHON=OFF "
	build_with_cmake
	make install
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	CURRENT_BUILD=libsigrokdecode
	pushd $STAGING_AREA/libsigrokdecode
	save_version_info
	git reset --hard
	git clean -xdf
	./autogen.sh
	./configure --prefix $STAGING_AREA_DEPS
	make $JOBS install
	popd
}

patch_qwt() {
	patch -p1 <<-EOF
--- a/qwtconfig.pri
+++ b/qwtconfig.pri
@@ -19,7 +19,7 @@ QWT_VERSION      = \$\${QWT_VER_MAJ}.\$\${QWT_VER_MIN}.\$\${QWT_VER_PAT}
 QWT_INSTALL_PREFIX = \$\$[QT_INSTALL_PREFIX]

 unix {
-    QWT_INSTALL_PREFIX    = /usr/local/qwt-\$\$QWT_VERSION-ma
+    QWT_INSTALL_PREFIX    = $STAGING_AREA_DEPS
     # QWT_INSTALL_PREFIX = /usr/local/qwt-\$\$QWT_VERSION-ma-qt-\$\$QT_VERSION
 }

@@ -42,7 +42,7 @@ QWT_INSTALL_LIBS      = \$\${QWT_INSTALL_PREFIX}/lib
 # runtime environment of designer/creator.
 ######################################################################

-QWT_INSTALL_PLUGINS   = \$\${QWT_INSTALL_PREFIX}/plugins/designer
+#QWT_INSTALL_PLUGINS   = \$\${QWT_INSTALL_PREFIX}/plugins/designer

 # linux distributors often organize the Qt installation
 # their way and QT_INSTALL_PREFIX doesn't offer a good
@@ -164,7 +164,7 @@ QWT_CONFIG     += QwtTests

 macx:!static:CONFIG(qt_framework, qt_framework|qt_no_framework) {

-    QWT_CONFIG += QwtFramework
+#    QWT_CONFIG += QwtFramework
 }

 ######################################################################
--- a/src/src.pro
+++ b/src/src.pro
@@ -36,6 +36,7 @@ contains(QWT_CONFIG, QwtDll) {
             QMAKE_LFLAGS_SONAME=
         }
     }
+    macx: QWT_SONAME=\$\${QWT_INSTALL_LIBS}/libqwt.dylib
 }
 else {
     CONFIG += staticlib
EOF
}


build_qwt() {
	echo "### Building qwt - branch qwt-multiaxes"
	CURRENT_BUILD=qwt
	pushd $STAGING_AREA/qwt
	save_version_info
	git clean -xdf
	git reset --hard
	patch_qwt
	$QMAKE INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
	make $JOBS
	make install
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	CURRENT_BUILD=libtinyiiod
	pushd $STAGING_AREA/libtinyiiod
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	build_with_cmake
	make install
	popd
}

build_kddock () {
	echo "### Building KDDockWidgets - version $KDDOCK_BRANCH"
	pushd $STAGING_AREA/KDDockWidgets
	CURRENT_BUILD_CMAKE_OPTS=""
	build_with_cmake
	make install
	popd
}

build_deps(){
	build_libserialport
	build_libiio
	build_libad9361
	build_libm2k
	build_gnuradio
	build_grscopy
	build_grm2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
	build_kddock
}

install_packages
export_paths
clone
generate_status_file
build_deps

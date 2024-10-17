#!/usr/bin/bash.exe
set -xe
# get the full directory path of the script
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

## Set STAGING
USE_STAGING=OFF
##

source $SCRIPT_DIR/mingw_toolchain.sh $USE_STAGING

install_packages() {
	SYSTEM_PKGS="\
		git\
		svn\
		vim\
		unzip\
		zip\
	"
	TOOLS_PKGS="\
		mingw-w64-${ARCH}-wget2\
		mingw-w64-${ARCH}-cmake\
		mingw-w64-${ARCH}-gcc\
		mingw-w64-${ARCH}-python3\
		mingw-w64-${ARCH}-python-mako\
		mingw-w64-${ARCH}-python-six\
		mingw-w64-${ARCH}-make\
		mingw-w64-${ARCH}-doxygen\
		mingw-w64-${ARCH}-pcre2\
		base-devel\
		mingw-w64-${ARCH}-autotools\
		libtool\
		mingw-w64-${ARCH}-boost
	"
	PACMAN_SYNC_DEPS="\
		mingw-w64-${ARCH}-fftw\
		mingw-w64-${ARCH}-orc\
		mingw-w64-${ARCH}-libxml2\
		mingw-w64-${ARCH}-libzip\
		mingw-w64-${ARCH}-fftw\
		mingw-w64-${ARCH}-libffi\
		mingw-w64-${ARCH}-glib2\
		mingw-w64-${ARCH}-glibmm\
		mingw-w64-${ARCH}-doxygen\
		mingw-w64-${ARCH}-qt5\
		mingw-w64-${ARCH}-angleproject\
		mingw-w64-${ARCH}-zlib\
		mingw-w64-${ARCH}-breakpad\
		mingw-w64-${ARCH}-libusb
	"

	if [ "$USE_STAGING" == "ON" ]; then
		mkdir -p $STAGING_DIR/var/lib/pacman/local
		mkdir -p $STAGING_DIR/var/lib/pacman/sync
		$PACMAN -Syuu bash filesystem mintty pacman
	fi

	pacman --noconfirm -S $SYSTEM_PKGS
	$PACMAN -S $TOOLS_PKGS
	$PACMAN -S $PACMAN_SYNC_DEPS
}

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	[ -d 'libserialport' ] || git clone --recursive https://github.com/cseci/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	[ -d 'libiio' ]		|| git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	[ -d 'libad9361' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	[ -d 'libm2k' ]		|| git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
	[ -d 'spdlog' ]		|| git clone --recursive https://github.com/gabime/spdlog.git -b $SPDLOG_BRANCH spdlog
	[ -d 'libsndfile' ]	|| git clone --recursive https://github.com/libsndfile/libsndfile -b $LIBSNDFILE_BRANCH libsndfile
	[ -d 'gr-scopy' ]	|| git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
	[ -d 'gr-m2k' ]		|| git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
	[ -d 'volk' ]		|| git clone --recursive https://github.com/gnuradio/volk.git -b $VOLK_BRANCH volk
	[ -d 'gnuradio' ]	|| git clone --recursive https://github.com/analogdevicesinc/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'libsigrokdecode' ] || git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
	[ -d 'libtinyiiod' ]	|| git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	[ -d 'iio-emu' ]	|| git clone --recursive https://github.com/analogdevicesinc/iio-emu -b $IIOEMU_BRANCH iio-emu
	[ -d 'KDDockWidgets' ] || git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	popd
}

create_build_status_file() {
	touch $BUILD_STATUS_FILE
	echo "Scopy2-MinGW" >> $BUILD_STATUS_FILE
	echo "Docker image built on $(date)" >> $BUILD_STATUS_FILE
	echo "Deps installed using pacman" >> $BUILD_STATUS_FILE
	echo "" >> $BUILD_STATUS_FILE
	echo "All explicitly installed packages on build machine" >> $BUILD_STATUS_FILE
	echo "" >> $BUILD_STATUS_FILE
	pacman --noconfirm -Qe >> $BUILD_STATUS_FILE
	echo "" >> $BUILD_STATUS_FILE
	echo "Deps built from sources" >> $BUILD_STATUS_FILE
	echo "" >> $BUILD_STATUS_FILE
}

clean_build_dir() {
	git clean -xdf
	rm -rf $BUILD_FOLDER
	mkdir $BUILD_FOLDER
	cd $BUILD_FOLDER
}

build_with_cmake() {
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	pushd $STAGING_AREA/$CURRENT_BUILD
	BUILD_FOLDER=$PWD/build
	clean_build_dir
	eval $CURRENT_BUILD_POST_CLEAN
	eval $CURRENT_BUILD_PATCHES

	$CMAKE $CURRENT_BUILD_CMAKE_OPTS $STAGING_AREA/$CURRENT_BUILD
	eval $CURRENT_BUILD_POST_CMAKE
	make $JOBS
	if [ "$INSTALL" == "ON" ];then
		make install
	fi
	eval $CURRENT_BUILD_POST_MAKE
	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE

	#clean deps folder
	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

	popd

	# clear vars
	CURRENT_BUILD_CMAKE_OPTS=""
	CURRENT_BUILD_POST_CLEAN=""
	CURRENT_BUILD_PATCHES=""
	CURRENT_BUILD_POST_CMAKE=""
	CURRENT_BUILD_POST_MAKE=""
	CURRENT_BUILD=""
}

build_libserialport(){
	CURRENT_BUILD=libserialport
	pushd $STAGING_AREA/$CURRENT_BUILD
	git clean -xdf

	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON

	./autogen.sh
	[ "$USE_STAGING" == "ON" ] && ./configure --prefix $STAGING_AREA_DEPS ${AUTOCONF_OPTS} || ./configure ${AUTOCONF_OPTS}
	make $JOBS
	[ "$INSTALL" == "ON" ] && make install

	#clean deps folder
	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd
}

build_libiio() {
	CURRENT_BUILD=libiio
	CURRENT_BUILD_CMAKE_OPTS="\
		${RC_COMPILER_OPT}\
		-DWITH_USB_BACKEND:BOOL=ON\
		-DWITH_SERIAL_BACKEND:BOOL=ON\
		-DCSHARP_BINDINGS:BOOL=OFF\
		-DPYTHON_BINDINGS:BOOL=OFF\
		-DHAVE_DNS_SD:BOOL=ON\
		-DENABLE_IPV6:BOOL=OFF\
		-DWITH_EXAMPLES:BOOL=ON\
	"
	build_with_cmake $1
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	CURRENT_BUILD=libad9361
	build_with_cmake $1
}

build_spdlog() {
	# [ -f "/usr/bin/x86_64-w64-mingw32-windres.exe" ] && rm -v /usr/bin/x86_64-w64-mingw32-windres.exe
	# ln -s /usr/bin/windres.exe /usr/bin/x86_64-w64-mingw32-windres.exe
	CURRENT_BUILD=spdlog
	CURRENT_BUILD_CMAKE_OPTS="\
		-DSPDLOG_BUILD_SHARED=ON\
		-DSPDLOG_BUILD_EXAMPLE=OFF\
		"
	build_with_cmake $1
}

build_libm2k() {
	CURRENT_BUILD=libm2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF\
		-DENABLE_CSHARP=OFF\
		-DBUILD_EXAMPLES=OFF\
		-DENABLE_TOOLS=ON\
		-DINSTALL_UDEV_RULES=OFF\
		"
	build_with_cmake $1
}

build_libsndfile() {
	CURRENT_BUILD=libsndfile
	CURRENT_BUILD_CMAKE_OPTS="\
	-DENABLE_EXTERNAL_LIBS=OFF\
	-DENABLE_MPEG=OFF\
	-DBUILD_PROGRAMS=OFF\
	-DBUILD_EXAMPLES=OFF\
	-DENABLE_CPACK=OFF\
	-DBUILD_SHARED_LIBS=OFF\
	-DBUILD_TESTING=OFF"
	build_with_cmake $1
}

build_volk() {
	CURRENT_BUILD=volk
	CURRENT_BUILD_POST_CLEAN="git submodule update --init ../cpu_features"
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_MODTOOL=OFF\
		-DENABLE_TESTING=OFF\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	build_with_cmake $1

}

build_gnuradio() {
	CURRENT_BUILD=gnuradio
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_DEFAULT=OFF\
		-DENABLE_GNURADIO_RUNTIME=ON\
		-DENABLE_GR_ANALOG=ON\
		-DENABLE_GR_BLOCKS=ON\
		-DENABLE_GR_FFT=ON\
		-DENABLE_GR_FILTER=ON\
		-DENABLE_VOLK=ON\
		-DENABLE_GR_IIO=ON\
		-DENABLE_POSTINSTALL=OFF\
		-DCMAKE_C_FLAGS=-fno-asynchronous-unwind-tables\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	build_with_cmake $1
}

build_grscopy() {
	CURRENT_BUILD=gr-scopy
	CURRENT_BUILD_CMAKE_OPTS="\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	build_with_cmake $1
}

build_grm2k() {
	CURRENT_BUILD=gr-m2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	build_with_cmake $1
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	CURRENT_BUILD=qwt
	pushd $STAGING_AREA/$CURRENT_BUILD
	git clean -xdf

patch -p1 <<-EOF
--- a/qwtconfig.pri
+++ b/qwtconfig.pri
@@ -24,7 +24,7 @@ unix {
 }
 
 win32 {
-    QWT_INSTALL_PREFIX    = ""
+    QWT_INSTALL_PREFIX    = "/mingw64"
     # QWT_INSTALL_PREFIX = C:/Qwt-\$\$QWT_VERSION-dev-qt-\$\$QT_VERSION
 }

EOF

	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON

	if [ "$USE_STAGING" == "ON" ]
	then
		$QMAKE INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
		make $JOBS
		if [ "$INSTALL" == "ON" ];then
			make INSTALL_ROOT=$STAGING_AREA_DEPS install
		fi
		cp -r $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
	else
		$QMAKE qwt.pro
		make $JOBS
		make install
	fi

	cp $STAGING_DIR/lib/qwt.dll $STAGING_DIR/bin/qwt.dll

	#clean deps folder
	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	CURRENT_BUILD=libsigrokdecode
	pushd $STAGING_AREA/$CURRENT_BUILD
	git reset --hard
	git clean -xdf
	patch -p1 < ${WORKFOLDER}/sigrokdecode-windows-fix.patch
	./autogen.sh

	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON

	if [ "$USE_STAGING" == "ON" ]
	then
		CPPFLAGS="-DLIBSIGROKDECODE_EXPORT=1" ./configure --prefix $STAGING_AREA_DEPS ${AUTOCONF_OPTS}
		LD_RUN_PATH=$STAGING_AREA_DEPS/lib make $JOBS
	else
		CPPFLAGS="-DLIBSIGROKDECODE_EXPORT=1" ./configure ${AUTOCONF_OPTS}
		make $JOBS
	fi

	if [ "$INSTALL" == "ON" ];then
		make install
	fi

	#clean deps folder
	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	CURRENT_BUILD=libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	build_with_cmake $1
}

build_kddock () {
	echo "### Building KDDockWidgets - version $KDDOCK_BRANCH"
	pushd $STAGING_AREA/KDDockWidgets
	CURRENT_BUILD_CMAKE_OPTS=""
	build_with_cmake $1
	popd
}

#
# Helper functions
#

build_deps() {
	install_packages
	create_build_status_file
	clone
	build_libserialport ON
	build_libiio ON
	build_libad9361 ON
	build_libm2k ON
	build_spdlog ON
	build_libsndfile ON
	build_volk ON
	build_gnuradio ON
	build_grscopy ON
	build_grm2k ON
	build_qwt ON
	build_libsigrokdecode ON
	build_libtinyiiod ON
	build_kddock ON
}


for arg in $@; do
	$arg
done

build_deps

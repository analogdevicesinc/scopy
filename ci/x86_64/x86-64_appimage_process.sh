#!/bin/bash
set -ex

## Set STAGING
USE_STAGING=OFF
##

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || echo "No source directory found"
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$CI_SCRIPT" == "ON" ]; then
	USE_STAGING=OFF
	SRC_DIR=$GITHUB_WORKSPACE
fi


export APPIMAGE=1

LIBIIO_VERSION=libiio-v0
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
IIOEMU_BRANCH=master

# default python version used in CI scripts, can be changed to match locally installed python
PYTHON_VERSION=python3.8

QT_LOCATION=/opt/Qt/5.15.2/gcc_64

STAGING_AREA=$SRC_SCRIPT/staging
QMAKE_BIN=$QT_LOCATION/bin/qmake
CMAKE_BIN=${STAGING_AREA}/cmake/bin/cmake
JOBS=-j14

APP_DIR_NAME=scopy.AppDir
APP_DIR=$SRC_SCRIPT/$APP_DIR_NAME
APP_IMAGE=$SRC_SCRIPT/Scopy-x86_64.AppImage

BUILD_STATUS_FILE=$SRC_SCRIPT/build-status

if [ "$USE_STAGING" == "ON" ]
	then
		echo -- USING STAGING FOLDER: $STAGING_AREA_DEPS
		STAGING_AREA_DEPS=$STAGING_AREA/dependencies
		export LD_LIBRARY_PATH=$STAGING_AREA_DEPS/lib:$QT_LOCATION/lib:$LD_LIBRARY_PATH
		CMAKE_OPTS=(\
			-DCMAKE_LIBRARY_PATH=$STAGING_AREA_DEPS \
			-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
			-DCMAKE_PREFIX_PATH=$QT_LOCATION\;$STAGING_AREA_DEPS \
			-DCMAKE_EXE_LINKER_FLAGS=-L$STAGING_AREA_DEPS\;-L$STAGING_AREA_DEPS/lib \
			-DCMAKE_SHARED_LINKER_FLAGS=-L$STAGING_AREA_DEPS\;-L$STAGING_AREA_DEPS/lib \
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
		echo  -- STAGING_DIR $STAGING_AREA_DEPS
	else
		echo -- NO STAGING: INSTALLING IN SYSTEM
		STAGING_AREA_DEPS=/usr/local
		export LD_LIBRARY_PATH=$QT_LOCATION/lib:$LD_LIBRARY_PATH:
		CMAKE_OPTS=(\
			-DCMAKE_PREFIX_PATH=$QT_LOCATION \
			-DCMAKE_BUILD_TYPE=RelWithDebInfo \
			-DCMAKE_VERBOSE_MAKEFILE=ON \
		)
fi

CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"
echo -- USING CMAKE COMMAND:
echo $CMAKE
echo -- USING QT: $QT_LOCATION
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
	popd
}

install_qt() {
	# installing Qt using the aqt tool https://github.com/miurahr/aqtinstall
	sudo pip3 install --no-cache-dir aqtinstall
	sudo python3 -m aqt install-qt --outputdir /opt/Qt linux desktop 5.15.2
}

download_tools() {
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}

	if [ ! -d cmake ];then
		wget https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2-linux-x86_64.tar.gz
		tar -xf cmake*.tar.gz && rm cmake*.tar.gz && mv cmake* cmake # unzip and rename
	fi

	# download tools for creating the AppDir and the AppImage
	if [ ! -f linuxdeploy-x86_64.AppImage ];then
		wget https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20240109-1/linuxdeploy-x86_64.AppImage
		chmod +x linuxdeploy-x86_64.AppImage
	fi

	if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ];then
		wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20240109-1/linuxdeploy-plugin-qt-x86_64.AppImage
		chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
	fi

	if [ ! -f linuxdeploy-plugin-appimage-x86_64.AppImage ];then
		wget https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/1-alpha-20230713-1/linuxdeploy-plugin-appimage-x86_64.AppImage
		chmod +x linuxdeploy-plugin-appimage-x86_64.AppImage
	fi

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
	if [ "$INSTALL" == "ON" ];then
		if [ "$USE_STAGING" == "ON" ]; then make install; else sudo make install; fi
	fi
	CURRENT_BUILD_CMAKE_OPTS=""
	echo "$(basename -a "$(git config --get remote.origin.url)") - \
	$(git rev-parse --abbrev-ref HEAD) - \
	$(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE
}

install_packages() {
	sudo DEBIAN_FRONTEND=noninteractive apt-get -y install

	sudo apt-get update
	sudo apt-get -y upgrade
	sudo apt-get -y --no-install-recommends install \
		$PYTHON_VERSION-full python3-pip lib$PYTHON_VERSION-dev python3-numpy \
		keyboard-configuration vim git wget unzip\
		g++ build-essential cmake curl autogen autoconf autoconf-archive pkg-config flex bison swig \
		subversion mesa-common-dev graphviz xserver-xorg gettext texinfo mm-common doxygen \
		libboost-all-dev libfftw3-bin libfftw3-dev libfftw3-3 liblog4cpp5v5 liblog4cpp5-dev \
		libxcb-xinerama0  libgmp3-dev libzip-dev libglib2.0-dev libglibmm-2.4-dev libsigc++-2.0-dev \
		libclang1-9 libmatio-dev liborc-0.4-dev libgl1-mesa-dev libserialport0 libserialport-dev \
		libusb-1.0 libusb-1.0-0 libusb-1.0-0-dev libavahi-client-dev libsndfile1-dev \
		libxkbcommon-x11-0 libqt5gui5 libncurses5 libtool libaio-dev libzmq3-dev libxml2-dev

	pip3 install --no-cache-dir mako
	pip3 install --no-cache-dir packaging
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

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	pushd $STAGING_AREA/libad9361
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
	build_with_cmake $1
	popd
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
	pushd $STAGING_AREA/gr-scopy
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

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	pushd $STAGING_AREA/qwt
	git clean -xdf
	sed -i 's|/usr/local/qwt-$$QWT_VERSION-ma|/usr/local|g' qwtconfig.pri

	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON

	if [ "$USE_STAGING" == "ON" ]
	then
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

	echo "$(basename -a "$(git config --get remote.origin.url)") - \
	$(git rev-parse --abbrev-ref HEAD) - \
	$(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE

	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	pushd $STAGING_AREA/libsigrokdecode
	git clean -xdf
	./autogen.sh

	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON

	if [ "$USE_STAGING" == "ON" ]
	then
		./configure --prefix $STAGING_AREA_DEPS
		LD_RUN_PATH=$STAGING_AREA_DEPS/lib make $JOBS
	else
		./configure
		make $JOBS
	fi

	if [ "$INSTALL" == "ON" ];then
		if [ "$USE_STAGING" == "ON" ]; then make install; else sudo make install; fi
	fi

	echo "$(basename -a "$(git config --get remote.origin.url)") - \
	$(git rev-parse --abbrev-ref HEAD) - \
	$(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE

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
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	[ -d 'iio-emu' ] || git clone --recursive https://github.com/analogdevicesinc/iio-emu -b $IIOEMU_BRANCH iio-emu
	pushd $STAGING_AREA/iio-emu
	build_with_cmake OFF
	popd
	popd
}

build_scopy() {
	echo "### Building scopy"
	pushd $SRC_DIR
	[ -f /home/runner/build-status ] && cp /home/runner/build-status $SRC_DIR/build-status
	[ $CI_SCRIPT ] && git config --global --add safe.directory $SRC_DIR
	CURRENT_BUILD_CMAKE_OPTS="\
		-DPYTHON_EXECUTABLE=/usr/bin/$PYTHON_VERSION
		"
	build_with_cmake OFF
	popd
}

create_appdir(){
	pushd ${STAGING_AREA}
	BUILD_FOLDER=$SRC_DIR/build
	EMU_BUILD_FOLDER=$STAGING_AREA/iio-emu/build
	PLUGINS=$BUILD_FOLDER/plugins/plugins
	SCOPY_DLL=$(find $BUILD_FOLDER -maxdepth 1 -type f -name "libscopy*")
	REGMAP_XMLS=$BUILD_FOLDER/plugins/regmap/xmls
	DAC_WAVEFORM_CSV=$SRC_DIR/plugins/dac/res/csv
	EMU_XMLS=$BUILD_FOLDER/plugins/emu_xml
	EMU_CONFIG=$SRC_DIR/resources/scopy_emu_options_config.json
	TRANSLATIONS_QM=$(find $BUILD_FOLDER/translations -type f -name "*.qm")
	STYLE_FOLDER=$BUILD_FOLDER/style
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STAGING_AREA_DEPS/lib:$QT_LOCATION/lib
	DLL_FOLDER=${STAGING_AREA}/dll_folder
	COPY_DEPS=${SRC_DIR}/ci/x86_64/copy-deps.sh
	export PATH=$QT_LOCATION:$PATH
	sudo ldconfig

	rm -rf $APP_DIR
	rm -rf $DLL_FOLDER
	mkdir $DLL_FOLDER
	cp $SCOPY_DLL $DLL_FOLDER
	cp $PLUGINS/*.so $DLL_FOLDER

	export QMAKE=$QMAKE_BIN # this is needed for deploy-plugin-qt.AppImage
	# inside a docker image you can't run an appimage executable without privileges
	# so the solution is to extract the appimage first and only then to run it
	export APPIMAGE_EXTRACT_AND_RUN=1
	${STAGING_AREA}/linuxdeploy-x86_64.AppImage \
		--appdir  $APP_DIR \
		--executable $SRC_DIR/build/scopy \
		--custom-apprun $SRC_DIR/ci/x86_64/AppRun \
		--desktop-file $SRC_DIR/ci/x86_64/scopy.desktop \
		--icon-file $SRC_DIR/gui/res/scopy.png \
		--deploy-deps-only $DLL_FOLDER \
		--plugin qt

	$COPY_DEPS "$DLL_FOLDER/*" $APP_DIR/usr/lib
	rm -rf $DLL_FOLDER
	cp $SCOPY_DLL $APP_DIR/usr/lib
	mkdir -p $APP_DIR/usr/lib/scopy/plugins
	cp $PLUGINS/*.so $APP_DIR/usr/lib/scopy/plugins

	cp $EMU_BUILD_FOLDER/iio-emu $APP_DIR/usr/bin
	cp ${STAGING_AREA_DEPS}/lib/tinyiiod.so* $APP_DIR/usr/lib

	# search for the python version linked by cmake and copy inside the appimage the same version
	FOUND_PYTHON_VERSION=$(grep 'PYTHON_VERSION' $SRC_DIR/build/CMakeCache.txt | awk -F= '{print $2}' | grep -o 'python[0-9]\+\.[0-9]\+')
	python_path=/usr/lib/$FOUND_PYTHON_VERSION
	cp -r $python_path $APP_DIR/usr/lib

	if [ -d $STAGING_AREA_DEPS/share/libsigrokdecode/decoders ]; then
		cp -r $STAGING_AREA_DEPS/share/libsigrokdecode/decoders $APP_DIR/usr/lib
	elif [ -d $STAGING_AREA/libsigrokdecode/decoders ];then
		cp -r $STAGING_AREA/libsigrokdecode/decoders $APP_DIR/usr/lib
	else
		echo  "No decoders for libsigrokdecode found"
		exit 1
	fi
	
	mkdir -p $APP_DIR/usr/lib/scopy/translations
	cp $TRANSLATIONS_QM $APP_DIR/usr/lib/scopy/translations
	
	cp -R $STYLE_FOLDER $APP_DIR/usr/lib/scopy/style

	if [ -d $REGMAP_XMLS ]; then
		cp -r $REGMAP_XMLS $APP_DIR/usr/lib/scopy/plugins
	fi

	cp -r $DAC_WAVEFORM_CSV $APP_DIR/usr/lib/scopy/plugins

	cp -r $EMU_XMLS $APP_DIR/usr/lib/scopy/plugins
	mkdir -p $APP_DIR/usr/lib/scopy/plugins/resources
	cp $EMU_CONFIG $APP_DIR/usr/lib/scopy/plugins/resources

	cp $STAGING_AREA_DEPS/lib/libspdlog.so* $APP_DIR/usr/lib
	cp -r $QT_LOCATION/plugins $APP_DIR/usr
	cp $QT_LOCATION/lib/libQt5XcbQpa.so* $APP_DIR/usr/lib
	cp $QT_LOCATION/lib/libQt5EglFSDeviceIntegration.so* $APP_DIR/usr/lib
	cp $QT_LOCATION/lib/libQt5DBus.so* $APP_DIR/usr/lib
	cp /usr/lib/x86_64-linux-gnu/libXdmcp.so* $APP_DIR/usr/lib
	cp /usr/lib/x86_64-linux-gnu/libbsd.so* $APP_DIR/usr/lib
	cp /usr/lib/x86_64-linux-gnu/libXau.so* $APP_DIR/usr/lib
	cp /usr/lib/x86_64-linux-gnu/libffi.so* $APP_DIR/usr/lib
	popd
}

create_appimage(){
	rm -rf $APP_IMAGE

	pushd ${STAGING_AREA}
	export APPIMAGE_EXTRACT_AND_RUN=1
	${STAGING_AREA}/linuxdeploy-plugin-appimage-x86_64.AppImage --appdir $APP_DIR
	mv Scopy*.AppImage $APP_IMAGE
	chmod +x $APP_IMAGE
	popd
}

generate_ci_envs(){
	$SRC_DIR/ci/general/gen_ci_envs.sh > $SRC_DIR/ci/x86_64/gh-actions.envs
}

# move the staging folder that contains the tools needed for the build to the known location
move_tools(){
	[ -d /home/runner/staging ] && mv /home/runner/staging $STAGING_AREA || echo "Staging folder not found or already moved"
	if [ ! -d $STAGING_AREA ]; then
		echo "Missing tools folder, downloading now"
		download_tools
	fi
}

move_appimage(){
	mv $APP_IMAGE $SRC_DIR
}


#
# Helper functions
#

build_deps(){
	clone
	download_tools
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
}

run_workflow(){
	[ "$CI_SCRIPT" == "ON" ] && move_tools || download_tools
	build_iio-emu
	build_scopy
	create_appdir
	create_appimage
	move_appimage
}

get_tools(){
	install_packages
	download_tools
}

generate_appimage(){
	download_tools
	build_iio-emu
	build_scopy
	create_appdir
	create_appimage
}

configure_system(){
	install_packages
	install_qt
	build_deps
	download_tools
}

for arg in $@; do
	$arg
done

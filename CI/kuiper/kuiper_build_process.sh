#!/bin/bash

set -ex
git config --global --add safe.directory $HOME/scopy
SRC_DIR=$(git rev-parse --show-toplevel)
source $SRC_DIR/CI/kuiper/kuiper_build_config.sh

echo -- USING CMAKE COMMAND:
echo $CMAKE
echo -- USING QT: $QT
echo -- USING QMAKE: $QMAKE_BIN

build_with_cmake() {
	BUILD_FOLDER=$PWD/build
	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	$CMAKE $CURRENT_BUILD_CMAKE_OPTS ../
	make $JOBS
	CURRENT_BUILD_CMAKE_OPTS=""
}

set_config_opts() {
	CPP="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-cpp"
	CC="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-gcc"
	CXX="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-g++"
	LD="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-ld"
	AS="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-as"
	AR="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-ar"
	RANLIB="${TOOLCHAIN_BIN}/${TOOLCHAIN_HOST}-ranlib"

	CFLAGS=" -I${SYSROOT}/include -I${SYSROOT}/include/arm-linux-gnueabihf -I${SYSROOT}/usr/include -I${SYSROOT}/usr/include/arm-linux-gnueabihf -I${TOOLCHAIN}/include- -fPIC"
	CPPFLAGS="-fexceptions ${CFLAGS}"
	LDFLAGS="-Wl,-rpath=XORIGIN -L${TOOLCHAIN}/arm-linux-gnueabihf/lib -L${TOOLCHAIN}/arm-linux-gnueabihf/libc/lib -L${TOOLCHAIN}/arm-linux-gnueabihf/libc/usr/lib -L${SYSROOT}/lib -L${SYSROOT}/usr/lib -L${SYSROOT}/usr/lib/arm-linux-gnueabihf -L${SYSROOT}/usr/lib/arm-linux-gnueabihf"

	CONFIG_OPTS=()
	CONFIG_OPTS+=("--prefix=${SYSROOT}")
	CONFIG_OPTS+=("--host=${TOOLCHAIN_HOST}")
	CONFIG_OPTS+=("--with-sysroot=${SYSROOT}")
	CONFIG_OPTS+=("CFLAGS=${CFLAGS}")
	CONFIG_OPTS+=("CPPFLAGS=${CPPFLAGS}")
	CONFIG_OPTS+=("LDFLAGS=${LDFLAGS}")
	CONFIG_OPTS+=("PKG_CONFIG=${SYSROOT}/usr/bin/arm-linux-gnueabihf-pkg-config" )
	CONFIG_OPTS+=("PKG_CONFIG_DIR=")
	CONFIG_OPTS+=("PKG_CONFIG_LIBDIR=${SYSROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${SYSROOT}/usr/share/pkgconfig:${SYSROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${SYSROOT}/usr/local/lib/pkgconfig")
	CONFIG_OPTS+=("PKG_CONFIG_SYSROOT=${SYSROOT}")
	CONFIG_OPTS+=("PKG_CONFIG_SYSROOT_DIR=${SYSROOT}")
	CONFIG_OPTS+=("PKG_CONFIG_PATH=${SYSROOT}/usr/bin/arm-linux-gnueabihf-pkg-config")
	CONFIG_OPTS+=("PKG_CONFIG_ALLOW_CROSS=1")
	CONFIG_OPTS+=("CPP=${CPP}")
	CONFIG_OPTS+=("CC=${CC}")
	CONFIG_OPTS+=("CXX=${CXX}")
	CONFIG_OPTS+=("LD=${LD}")
	CONFIG_OPTS+=("AS=${AS}")
	CONFIG_OPTS+=("AR=${AR}")
	CONFIG_OPTS+=("RANLIB=${RANLIB}")
}

install_packages() {
	sudo apt update
	sudo apt install -y build-essential cmake unzip gfortran gcc git bison libtool \
		python3 pip gperf pkg-config gdb-multiarch g++ flex texinfo gawk openssl \
		pigz libncurses-dev autoconf automake tar figlet liborc-0.4-dev* patchelf libc6-dev-armhf-cross squashfs-tools
	pip install mako
}

download_cmake() {
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d cmake ];then
		wget ${CMAKE_DOWNLOAD_LINK}
		# unzip and rename
		tar -xvf cmake*.tar.gz && rm cmake*.tar.gz && mv cmake* cmake
	else
		echo "Cmake already downloaded"
	fi
	popd
}

download_crosscompiler(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d cross-pi-gcc ];then
		wget --progress=dot:giga ${CROSSCOMPILER_DOWNLOAD_LINK}
		# unzip and rename
		tar -xf cross-gcc-*.tar.gz && rm cross-gcc-*.tar.gz && mv cross-pi-* cross-pi-gcc
	else
		echo "Crosscompiler already downloaded"
	fi
	popd
}

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
	[ -d 'gnuradio' ]	|| git clone --recursive https://github.com/gnuradio/gnuradio.git -b $GNURADIO_BRANCH gnuradio
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'libsigrokdecode' ] || git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
	[ -d 'libtinyiiod' ]	|| git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	popd
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
	sudo make install
	popd
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	pushd $STAGING_AREA/libad9361
	build_with_cmake
	sudo make install
	popd
}

build_spdlog() {
	echo "### Building spdlog - branch $SPDLOG_BRANCH"
	pushd $STAGING_AREA/spdlog
	CURRENT_BUILD_CMAKE_OPTS="-DSPDLOG_BUILD_SHARED=ON"
	build_with_cmake
	sudo make install
	popd
}

build_libm2k() {
	echo "### Building libm2k - branch $LIBM2K_BRANCH"
	pushd $STAGING_AREA/libm2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DENABLE_CSHARP=OFF \
		-DBUILD_EXAMPLES=ON \
		-DENABLE_TOOLS=OFF \
		-DINSTALL_UDEV_RULES=OFF \
		-DENABLE_LOG=OFF
		"
	build_with_cmake
	sudo make install
	popd
}

build_volk() {
	echo "### Building volk - branch $VOLK_BRANCH"
	pushd $STAGING_AREA/volk
	build_with_cmake
	sudo make install
	popd
}

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"
	pushd $STAGING_AREA/gnuradio
	CURRENT_BUILD_CMAKE_OPTS="\
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
	sudo make install
	popd
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
	pushd $STAGING_AREA/gr-scopy
	build_with_cmake
	sudo make install
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
	sudo make install
	popd
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	pushd $STAGING_AREA/qwt
	git clean -xdf
	sed -i 's|/usr/local/qwt-$$QWT_VERSION-ma|/usr/local|g' qwtconfig.pri
	$QMAKE_BIN INCLUDEPATH=$SYSROOT/include LIBS=-L$SYSROOT/lib qwt.pro
	make $JOBS
	patchelf --force-rpath --set-rpath \$ORIGIN $STAGING_AREA/qwt/lib/libqwt.so
	sudo make INSTALL_ROOT=$SYSROOT install
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	set_config_opts
	pushd $STAGING_AREA/libsigrokdecode
	git clean -xdf
	./autogen.sh
	./configure "${CONFIG_OPTS[@]}"
	make $JOBS
	patchelf --force-rpath --set-rpath \$ORIGIN $STAGING_AREA/libsigrokdecode/.libs/libsigrokdecode.so
	sudo make install
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	pushd $STAGING_AREA/libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	build_with_cmake
	sudo make install
	popd
}

build_iio-emu(){
	echo "### Building iio-emu - branch $IIOEMU_BRANCH"
	pushd $STAGING_AREA
	[ -d 'iio-emu' ] || git clone --recursive https://github.com/analogdevicesinc/iio-emu -b $IIOEMU_BRANCH iio-emu
	pushd $STAGING_AREA/iio-emu
	build_with_cmake
	sudo make install
	popd
	popd
}

build_scopy() {
	echo "### Building scopy"
	pushd $SRC_DIR
	CURRENT_BUILD_CMAKE_OPTS="\
		-DCLONE_IIO_EMU=OFF
		"
	build_with_cmake
	popd
}

build_deps(){
	build_libiio
	build_libad9361
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

create_appdir(){

	BUILD_FOLDER=$SRC_DIR/build
	EMU_BUILD_FOLDER=$STAGING_AREA/iio-emu/build
	COPY_DEPS=$SRC_DIR/CI/kuiper/copy-deps.sh

	rm -rf $APP_DIR
	mkdir $APP_DIR
	mkdir -p $APP_DIR/usr/bin
	mkdir -p $APP_DIR/usr/lib
	mkdir -p $APP_DIR/usr/share/applications
	mkdir -p $APP_DIR/usr/share/icons/hicolor/512x512

	cp $APP_RUN $APP_DIR
	cp $APP_DESKTOP $APP_DIR
	cp $SRC_DIR/resources/Scopy.png $APP_DIR
	cp $SRC_DIR/resources/Scopy.png $APP_DIR/usr/share/icons/hicolor/512x512
	cp $APP_DESKTOP $APP_DIR/usr/share/applications

	cp $EMU_BUILD_FOLDER/iio-emu $APP_DIR/usr/bin
	cp $BUILD_FOLDER/scopy $APP_DIR/usr/bin

	$COPY_DEPS $APP_DIR/usr/bin/scopy $APP_DIR/usr/lib
	$COPY_DEPS $APP_DIR/usr/bin/iio-emu $APP_DIR/usr/lib


	cp -r $QT_LOCATION/plugins $APP_DIR/usr
	cp -r $SYSROOT/lib/python3.9 $APP_DIR/usr/lib
	cp -r $SYSROOT/share/libsigrokdecode/decoders  $APP_DIR/usr/lib

	cp $QT_LOCATION/lib/libQt5XcbQpa.so* $APP_DIR/usr/lib
	cp $QT_LOCATION/lib/libQt5EglFSDeviceIntegration.so* $APP_DIR/usr/lib
	cp $QT_LOCATION/lib/libQt5DBus.so* $APP_DIR/usr/lib


	cp $SYSROOT/lib/arm-linux-gnueabihf/libstdc++.so* $APP_DIR/usr/lib
	cp $SYSROOT/lib/arm-linux-gnueabihf/libc.so* $APP_DIR/usr/lib
	cp $SYSROOT/lib/arm-linux-gnueabihf/libdl.so* $APP_DIR/usr/lib
	cp $SYSROOT/lib/arm-linux-gnueabihf/libpthread.so* $APP_DIR/usr/lib
	cp $SYSROOT/lib/arm-linux-gnueabihf/libGLESv2.so* $APP_DIR/usr/lib

	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/ld-linux-armhf.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libarmmem-v7l.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libm.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libgcc_s.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libdl.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libmd.so* $APP_DIR/usr/lib
	cp $SYSROOT/lib/arm-linux-gnueabihf/libbsd.so* $APP_DIR/usr/lib
	cp $SYSROOT/lib/arm-linux-gnueabihf/libXdmcp.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libXau.so* $APP_DIR/usr/lib
	cp $SYSROOT/usr/lib/arm-linux-gnueabihf/libxcb.so* $APP_DIR/usr/lib
}


create_appimage(){
	rm -rf $APP_IMAGE
	mksquashfs $APP_DIR  $APP_SQUASHFS -root-owned -noappend
	cat $RUNTIME_ARMHF >> $APP_IMAGE
	cat $APP_SQUASHFS >> $APP_IMAGE
	chmod a+x $APP_IMAGE
}

# move the sysroot from the home of the docker container to the known location
move_sysroot(){
	mkdir -p $STAGING_AREA
	[ -d /home/runner/sysroot ] && sudo mv /home/runner/sysroot $SYSROOT
}

# move and rename the AppImage artifact
move_appimage(){
	mv $APP_IMAGE $SRC_DIR/Scopy1-armhf.AppImage
}

generate_ci_envs(){
	$SRC_DIR/CI/appveyor/gen_ci_envs.sh > $SRC_DIR/CI/kuiper/gh-actions.envs
}

for arg in $@; do
	$arg
done

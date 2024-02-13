#!/bin/bash

set -ex
SRC_DIR=$(git rev-parse --show-toplevel)
source $SRC_DIR/ci/kuiper/kuiper_build_config.sh

install_packages(){
	sudo apt install -y build-essential cmake unzip gfortran gcc git bison \
		python gperf pkg-config gdb-multiarch g++ flex texinfo gawk openssl \
		pigz libncurses-dev autoconf automake tar figlet
}

#Download and extract QT Source (QT 5.15.2)
download_qt(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	wget ${QT_DOWNLOAD_LINK}
	tar -xf qt-everywhere-src-*.tar.xz && rm qt-everywhere-src-*.tar.xz && mv qt-everywhere-src-* qt-everywhere-src # unzip and rename
	cd qt-everywhere-src
	patch -p1 < $SRC_DIR/ci/kuiper/qt_patch.patch # Patch QT Source
	popd
}

download_crosscompiler(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	wget ${CROSSCOMPILER_DOWNLOAD_LINK}
	tar -xf cross-gcc-*.tar.gz && rm cross-gcc-*.tar.gz && mv cross-pi-* cross-pi-gcc # unzip and rename
	popd
}

build_qt(){
	mkdir -p $STAGING_AREA/build-qt5.15.2 && cd $STAGING_AREA/build-qt5.15.2
	../qt-everywhere-src/configure \
	-v \
	-release \
	-opensource \
	-confirm-license \
	-sysroot $SYSROOT \
	-prefix $QT_SYSTEM_LOCATION \
	-extprefix $QT_BUILD_LOCATION \
	-eglfs \
	-opengl es2 \
	-device linux-rasp-pi4-v3d-g++ \
	-device-option CROSS_COMPILE=$CROSS_COMPILER/bin/arm-linux-gnueabihf- \
	-skip qtscript \
	-skip qtwayland \
	-skip qtwebengine \
	-nomake tests \
	-make libs \
	-pkg-config \
	-no-use-gold-linker \
	-recheck \
	-xcb \
	-xcb-xlib \
	-bundled-xcb-xinput \
	-qt-pcre \
	-qpa eglfs \
	-L$SYSROOT/usr/lib/arm-linux-gnueabihf -I$SYSROOT/usr/include/arm-linux-gnueabihf

	make -j14
	sudo make install #installs to $QT_BUILD_LOCATION
}

for arg in $@; do
	$arg
done

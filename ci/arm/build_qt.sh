#!/bin/bash

set -ex
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SRC_SCRIPT/arm_build_config.sh $1

install_packages(){
	sudo apt install -y build-essential cmake unzip gfortran gcc git bison \
		gperf pkg-config gdb-multiarch g++ flex texinfo gawk openssl \
		pigz libncurses-dev autoconf automake tar figlet libclang-dev
}

# Download and extract QT Source (QT 5.15.2 for armhf and QT 5.15.10 for arm64)
download_qt(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d qt-everywhere-src ];then
		wget --progress=dot:giga ${QT_DOWNLOAD_LINK}
		tar -xf qt-everywhere-*.tar.xz && rm qt-everywhere-*.tar.xz && mv qt-everywhere-* qt-everywhere-src # unzip and rename
		cd qt-everywhere-src

		# Patch QT Source
		if [ $TOOLCHAIN_HOST == "aarch64-linux-gnu"  ]; then
			patch -p1 < $SRC_SCRIPT/qt_patch_arm64.patch
		elif [ $TOOLCHAIN_HOST == "arm-linux-gnueabihf" ]; then
			patch -p1 < $SRC_SCRIPT/qt_patch_arm32.patch
		fi
	else
		echo "QT already downloaded"
	fi
	popd
}

download_crosscompiler(){
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d cross-pi-gcc ];then
		wget --progress=dot:giga ${CROSSCOMPILER_DOWNLOAD_LINK}
		tar -xf cross-gcc-*.tar.gz && rm cross-gcc-*.tar.gz && mv cross-pi-* cross-pi-gcc # unzip and rename
	else
		echo "Crosscompiler already downloaded"
	fi
	popd
}

build(){
	mkdir -p "$STAGING_AREA"/build-qt && cd "$STAGING_AREA"/build-qt
	../qt-everywhere-src/configure \
	-v \
	-release \
	-opensource \
	-confirm-license \
	-sysroot "$SYSROOT" \
	-prefix "$QT_SYSTEM_LOCATION" \
	-extprefix "$QT_BUILD_LOCATION" \
	-egl \
	-eglfs \
	-reduce-exports \
	-opengl desktop \
	-device "$QT_BUILD_DEVICE"\
	-device-option CROSS_COMPILE="$CROSS_COMPILER"/bin/"$TOOLCHAIN_HOST"- \
	-skip qtandroidextras \
	-skip qtcharts \
	-skip qtcanvas3d \
	-skip qtdatavis3d \
	-skip qtdoc \
	-skip qtgamepad \
	-skip qtgraphicaleffects \
	-skip qtlocation \
	-skip qtlottie \
	-skip qtmacextras \
	-skip qtnetworkauth \
	-skip qtpurchasing \
	-skip qtquick3d \
	-skip qtquickcontrols \
	-skip qtquickcontrols2 \
	-skip qtquicktimeline \
	-skip qtremoteobjects \
	-skip qtscript \
	-skip qtsensors \
	-skip qtspeech \
	-skip qttranslations \
	-skip qtvirtualkeyboard \
	-skip qtwebchannel \
	-skip qtwebengine \
	-skip qtwebglplugin \
	-skip qtwebsockets \
	-skip qtwebview \
	-skip qtwinextras \
	-skip qtxmlpatterns\
	-nomake examples -no-compile-examples \
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
	-L"$SYSROOT"/usr/lib/"$TOOLCHAIN_HOST" -I"$SYSROOT"/usr/include/"$TOOLCHAIN_HOST"

	make -j14
	sudo make install # installs to $QT_BUILD_LOCATION
}

for arg in "${@:2}"; do
	$arg
done

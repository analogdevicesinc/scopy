#!/bin/bash -xe

# Cross-compile Qt6 for armhf from x86_64
# ========================================
# Two-stage build: host tools (x86_64) then cross-compile (armhf)
# Uses Kuiper Linux sysroot for armhf headers/libraries
#
# Prerequisites:
#   - arm_cross_build_process.sh install_packages (host tools)
#   - create_sysroot_armhf.sh (sysroot with dev packages)
#
# Output:
#   - Host Qt6 at /opt/Qt/6.8.3/gcc_64
#   - Cross Qt6 at /opt/Qt/6.8.3/armhf
#   - Tarball: qt6-armhf-cross-installed.tar.gz
#
# Usage:
#   ./build_qt6_armhf_cross.sh                 # full build
#   ./build_qt6_armhf_cross.sh download        # download source only
#   ./build_qt6_armhf_cross.sh build_host      # host tools only
#   ./build_qt6_armhf_cross.sh configure build install  # cross steps
#   ./build_qt6_armhf_cross.sh package         # create tarball

SRC_SCRIPT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
STAGING_AREA=$SRC_SCRIPT/staging
SYSROOT=${SYSROOT:-$STAGING_AREA/sysroot}
TOOLCHAIN_HOST=arm-linux-gnueabihf
JOBS=-j$(nproc)

QT_VERSION=6.8.3
QT_DOWNLOAD_LINK=https://download.qt.io/archive/qt/6.8/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz
QT_SRC=$STAGING_AREA/qt-everywhere-src
QT_HOST_BUILD=$STAGING_AREA/build-qt6-host
QT_CROSS_BUILD=$STAGING_AREA/build-qt6-armhf
QT_HOST_PREFIX=/opt/Qt/${QT_VERSION}/gcc_64
QT_CROSS_PREFIX=/opt/Qt/${QT_VERSION}/armhf

QT_SKIP_MODULES="\
	-skip qtandroidextras \
	-skip qtcharts \
	-skip qtdatavis3d \
	-skip qtdoc \
	-skip qtgamepad \
	-skip qtlocation \
	-skip qtlottie \
	-skip qtnetworkauth \
	-skip qtquick3d \
	-skip qtgraphs \
	-skip qtquick3dphysics \
	-skip qtpositioning \
	-skip qtquickcontrols \
	-skip qtquicktimeline \
	-skip qtremoteobjects \
	-skip qtsensors \
	-skip qtspeech \
	-skip qttranslations \
	-skip qtvirtualkeyboard \
	-skip qtwebchannel \
	-skip qtwebengine \
	-skip qtwebsockets \
	-skip qtwebview"

download() {
	echo "########## DOWNLOAD QT6 SOURCE ##########"
	mkdir -p ${STAGING_AREA}
	pushd ${STAGING_AREA}
	if [ ! -d qt-everywhere-src ]; then
		wget --progress=dot:giga ${QT_DOWNLOAD_LINK}
		tar -xf qt-everywhere-*.tar.xz && rm qt-everywhere-*.tar.xz && mv qt-everywhere-* qt-everywhere-src
	else
		echo "Qt6 source already downloaded"
	fi
	popd
}

build_host() {
	echo "########## BUILD QT6 HOST (x86_64) ##########"
	echo "Qt6 cross-compilation requires host tools (moc, rcc, uic)"
	mkdir -p ${QT_HOST_BUILD} && cd ${QT_HOST_BUILD}

	${QT_SRC}/configure \
		-release \
		-opensource \
		-confirm-license \
		-prefix "$QT_HOST_PREFIX" \
		-nomake examples \
		-nomake tests \
		-opengl desktop \
		-xcb \
		${QT_SKIP_MODULES}

	cmake --build . --parallel $(nproc)
	sudo cmake --install .
}

configure() {
	echo "########## CONFIGURE QT6 CROSS (armhf) ##########"
	mkdir -p ${QT_CROSS_BUILD} && cd ${QT_CROSS_BUILD}

	export PKG_CONFIG_ALLOW_CROSS=1
	export PKG_CONFIG_SYSROOT_DIR=${SYSROOT}
	export PKG_CONFIG_LIBDIR="${SYSROOT}/usr/lib/${TOOLCHAIN_HOST}/pkgconfig:${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig:${SYSROOT}/usr/local/lib/pkgconfig"

	cmake -GNinja ${QT_SRC} \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=${QT_CROSS_PREFIX} \
		-DQT_HOST_PATH=${QT_HOST_PREFIX} \
		-DCMAKE_SYSTEM_NAME=Linux \
		-DCMAKE_SYSTEM_PROCESSOR=arm \
		-DCMAKE_C_COMPILER=${TOOLCHAIN_HOST}-gcc \
		-DCMAKE_CXX_COMPILER=${TOOLCHAIN_HOST}-g++ \
		-DCMAKE_C_FLAGS="-march=armv7-a -mfloat-abi=hard -mfpu=neon" \
		-DCMAKE_CXX_FLAGS="-march=armv7-a -mfloat-abi=hard -mfpu=neon" \
		-DCMAKE_SYSROOT=${SYSROOT} \
		-DCMAKE_FIND_ROOT_PATH="${SYSROOT}" \
		-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
		-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
		-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
		-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
		-DFEATURE_opengl_desktop=OFF \
		-DFEATURE_opengles2=ON \
		-DFEATURE_eglfs=ON \
		-DFEATURE_xcb=ON \
		-DBUILD_EXAMPLES=OFF \
		-DBUILD_TESTING=OFF \
		-DQT_BUILD_EXAMPLES=OFF \
		-DQT_BUILD_TESTS=OFF \
		-DBUILD_qtquick3d=OFF \
		-DBUILD_qtquick3dphysics=OFF \
		-DBUILD_qtgraphs=OFF \
		-DBUILD_qtcharts=OFF \
		-DBUILD_qtdatavis3d=OFF \
		-DBUILD_qtdoc=OFF \
		-DBUILD_qtlocation=OFF \
		-DBUILD_qtlottie=OFF \
		-DBUILD_qtnetworkauth=OFF \
		-DBUILD_qtpositioning=OFF \
		-DBUILD_qtquicktimeline=OFF \
		-DBUILD_qtremoteobjects=OFF \
		-DBUILD_qtsensors=OFF \
		-DBUILD_qtspeech=OFF \
		-DBUILD_qttranslations=OFF \
		-DBUILD_qtvirtualkeyboard=OFF \
		-DBUILD_qtwebchannel=OFF \
		-DBUILD_qtwebengine=OFF \
		-DBUILD_qtwebsockets=OFF \
		-DBUILD_qtwebview=OFF \
		-DBUILD_qtgrpc=OFF \
		-DBUILD_qtcoap=OFF \
		-DBUILD_qtmqtt=OFF \
		-DBUILD_qthttpserver=OFF \
		-DBUILD_qtquickeffectmaker=OFF \
		-DBUILD_qtactiveqt=OFF
}

build() {
	echo "########## BUILD QT6 CROSS (armhf) ##########"
	cd ${QT_CROSS_BUILD}
	cmake --build . --parallel $(nproc)
}

install() {
	echo "########## INSTALL QT6 CROSS (armhf) ##########"
	cd ${QT_CROSS_BUILD}
	sudo cmake --install .

	echo "########## POST-INSTALL: Create armhf mkspec (D15) ##########"
	sudo cp -r ${QT_CROSS_PREFIX}/mkspecs/linux-arm-gnueabi-g++ ${QT_CROSS_PREFIX}/mkspecs/linux-arm-gnueabihf-g++
	sudo sed -i 's/arm-linux-gnueabi-/arm-linux-gnueabihf-/g' ${QT_CROSS_PREFIX}/mkspecs/linux-arm-gnueabihf-g++/qmake.conf

	echo "########## POST-INSTALL: Fix target_qt.conf (D15, D16) ##########"
	sudo sed -i "s|Sysroot=.*|Sysroot=/opt/sysroot/armhf|" ${QT_CROSS_PREFIX}/bin/target_qt.conf
	sudo sed -i 's|TargetSpec=linux-g++|TargetSpec=linux-arm-gnueabihf-g++|' ${QT_CROSS_PREFIX}/bin/target_qt.conf
	sudo sed -i 's|SysrootifyPrefix=true|SysrootifyPrefix=false|' ${QT_CROSS_PREFIX}/bin/target_qt.conf
}

package() {
	echo "########## PACKAGE QT6 CROSS (armhf) ##########"
	sudo tar -czf ${SRC_SCRIPT}/qt6-armhf-cross-installed.tar.gz -C / opt/Qt/${QT_VERSION}/armhf
	echo "Created: ${SRC_SCRIPT}/qt6-armhf-cross-installed.tar.gz"
	ls -lh ${SRC_SCRIPT}/qt6-armhf-cross-installed.tar.gz

	echo "########## PACKAGE QT6 HOST (x86_64) ##########"
	sudo tar -czf ${SRC_SCRIPT}/qt6-host-installed.tar.gz -C / opt/Qt/${QT_VERSION}/gcc_64
	echo "Created: ${SRC_SCRIPT}/qt6-host-installed.tar.gz"
	ls -lh ${SRC_SCRIPT}/qt6-host-installed.tar.gz
}

full_build() {
	download
	build_host
	configure
	build
	install
	package
}

if [ $# -eq 0 ]; then
	full_build
else
	for arg in "$@"; do
		$arg
	done
fi

#!/bin/bash
set -e
set -x
export PS4='+(${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

HOME="/home/analog"
STAGING_AREA=$HOME"/staging"
STAGING_AREA_DEPS=$STAGING_AREA"/dependencies"
JOBS=1

mkdir -p $STAGING_AREA_DEPS
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STAGING_AREA_DEPS/lib

apt-get install libsndfile-dev -y
apt-get install libbost1.71-all-dev

build_with_cmake() {
	echo $PWD
	BUILD_FOLDER=$PWD/build
	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER
	cmake \
		-DCMAKE_LIBRARY_PATH=$STAGING_AREA_DEPS \
		-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
		-DCMAKE_PREFIX_PATH=$STAGING_AREA_DEPS \
		-DCMAKE_EXE_LINKER_FLAGS="-L$STAGING_AREA_DEPS -L$STAGING_AREA_DEPS/lib" \
		-DCMAKE_SHARED_LINKER_FLAGS="-L$STAGING_AREA_DEPS -L$STAGING_AREA_DEPS/lib" \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		"$@"
	make -j$JOBS
	make -j$JOBS install
	ldconfig
}

clone() {
	echo "#######CLONE#######"
	pushd $STAGING_AREA
	git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b master libm2k
	git clone --recursive https://github.com/gabime/spdlog.git -b v1.x spdlog
	git clone --recursive https://github.com/gnuradio/volk.git -b main volk
	git clone --recursive https://github.com/gnuradio/gnuradio.git -b maint-3.10 gnuradio
	git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b 3.10 gr-scopy
	git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b master gr-m2k
	git clone --recursive https://github.com/cseci/qwt.git -b qwt-multiaxes qwt
	git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b master libsigrokdecode
	git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b master libtinyiiod
	git clone --recursive https://github.com/analogdevicesinc/scopy -b ci-gr-3.10 scopy
	popd
}

build_libm2k() {
	echo "#######build_libm2k#######"
	pushd $STAGING_AREA/libm2k
	build_with_cmake -DENABLE_PYTHON=OFF -DENABLE_TOOLS=ON ../
	popd
}

build_spdlog() {
	echo "#######build_spdlog#######"
	pushd $STAGING_AREA/spdlog
	build_with_cmake -DSPDLOG_BUILD_SHARED=ON ../
	popd
}

build_volk() {
	echo "#######build_volk#######"
	pushd $STAGING_AREA/volk
	build_with_cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DPYTHON_EXECUTABLE=/usr/bin/python3 ../
	popd
}

build_gnuradio() {
	echo "#######build_gnuradio#######"
	pushd $STAGING_AREA/gnuradio
	build_with_cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DPYTHON_EXECUTABLE=/usr/bin/python3 \
		-DENABLE_GR_QTGUI=OFF \
		-DENABLE_GR_DTV=OFF \
		-DENABLE_GR_DIGITAL=OFF \
		-DENABLE_GR_VOCODER=OFF \
		-DENABLE_GR_ZEROMQ=OFF \
		-DENABLE_GR_NETWORK=OFF \
		-DENABLE_GR_WAVELET=OFF \
		-DENABLE_GR_FEC=OFF \
		-DENABLE_GR_VIDEO_SDL=OFF \
		-DENABLE_GR_PDU=OFF \
		-DENABLE_GR_CTRLPORT=OFF \
		-DENABLE_GR_CHANNELS=OFF \
		-DENABLE_GR_AUDIO=OFF \
		-DENABLE_TESTING=OFF \
		-DENABLE_DOXYGEN=OFF \
		-DENABLE_POSTINSTALL=OFF ../
	popd
}

build_gr_scopy() {
	echo "#######build_gr_scopy#######"
	pushd $STAGING_AREA/gr-scopy
	build_with_cmake -DWITH_PYTHON=OFF ../
	popd
}

build_gr_m2k() {
	echo "#######build_gr_m2k#######"
	pushd $STAGING_AREA/gr-m2k
	build_with_cmake -DWITH_PYTHON=OFF ../
	popd
}

build_qwt() {
	echo "#######build_qwt#######"
	pushd $STAGING_AREA/qwt
	qmake INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
	make
	make INSTALL_ROOT=$STAGING_AREA_DEPS install
	ldconfig
	popd
}

copy_qwt() {
	cp -r $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
}

build_libsigrokdecode() {
	echo "#######build_libsigrokdecode#######"
	pushd $STAGING_AREA/libsigrokdecode
	./autogen.sh
	./configure --prefix $STAGING_AREA_DEPS
	make
	make install
	ldconfig
	popd
}

build_libtinyiiod() {
	echo "#######build_libtinyiiod#######"
	pushd $STAGING_AREA/libtinyiiod
	build_with_cmake ../
	popd
}

build_scopy() {
	echo "#######build_scopy#######"
	pushd $STAGING_AREA/scopy
	build_with_cmake ../
	popd
}

test_scopy() {
	echo "#######TEST_SCOPY#######"
	pushd $STAGING_AREA/scopy/build
	./scopy
	popd
}

clone
build_libm2k
build_spdlog
build_volk
build_gnuradio
build_gr_scopy
build_gr_m2k
build_qwt
copy_qwt
build_libsigrokdecode
build_libtinyiiod
build_scopy
test_scopy

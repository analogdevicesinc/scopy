#!/bin/bash
set -xe

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || echo "No source directory found"
SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SRC_DIR/ci/android/android_toolchain.sh

CLEAN_BEFORE_BUILD=ON


install_packages() {
    sudo apt-get update
    sudo apt-get install -y groff
}

install_jdk() {
    mkdir -p ${STAGING_AREA}
    pushd ${STAGING_AREA}

            # check jdk version =-=-=-=-=-=


    # we're using gradle 6.3 so we need to use jdk 14 at most
    # https://docs.gradle.org/current/userguide/compatibility.html

    wget https://download.java.net/openjdk/jdk14/ri/openjdk-14+36_linux-x64_bin.tar.gz
    tar xf openjdk-*.tar.gz
    mv jdk-* jdk
    rm openjdk-*.tar.gz
    popd
}

download_sdk(){
    mkdir -p ${STAGING_AREA}
    pushd ${STAGING_AREA}
    # Download and set up Android SDK command line tools
    if [ ! -d ${STAGING_AREA}/tools ]; then
        wget https://dl.google.com/android/repository/commandlinetools-linux-6200805_latest.zip
        unzip commandlinetools*
        rm commandlinetools*.zip
    fi

    # Install Android SDK tools
    yes | $SDK_MANAGER --sdk_root=${ANDROID_SDK_ROOT} --licenses
    yes | $SDK_MANAGER --sdk_root=${ANDROID_SDK_ROOT} "platforms;android-$API"
    yes | $SDK_MANAGER --sdk_root=${ANDROID_SDK_ROOT} "ndk;$NDK_VERSION"
    yes | $SDK_MANAGER --sdk_root=${ANDROID_SDK_ROOT} "platform-tools"
    yes | $SDK_MANAGER --sdk_root=${ANDROID_SDK_ROOT} "build-tools;$ANDROID_SDK_BUILD_TOOLS"
    yes | $SDK_MANAGER --sdk_root=${ANDROID_SDK_ROOT} "cmdline-tools;latest"

    # Symlink adb binary
    # sudo ln -s /home/$USER/Android/Sdk/platform-tools/adb /usr/local/bin/adb

    popd
}

download_cmake() {
    mkdir -p ${STAGING_AREA}
    pushd ${STAGING_AREA}
    if [ ! -d cmake ];then
        wget ${CMAKE_DOWNLOAD_LINK}
        # unzip and rename
        tar -xf cmake*.tar.gz && rm cmake*.tar.gz && mv cmake* cmake
    else
        echo "Cmake already downloaded"
    fi
    popd
}

strip(){
    SO_FILES=$(find . -type f -name "*.so")
    #     if [[ -n "$SO_FILES" ]]; then
    #         $STRIP --strip-unneeded $SO_FILES
    #     fi
}

meson_flag_list(){
    while (( "$#" )); do
        echo -n "'$1'";
        if [ $# -gt 1 ]; then
            echo -n ",";
        fi
        shift
    done
}

create_build_status_file() {
    touch $BUILD_STATUS_FILE
    echo "NDK - $NDK_VERSION" >> $BUILD_STATUS_FILE
    echo "ANDROID API - $API" >> $BUILD_STATUS_FILE
    echo "ABI - $ABI" >> $BUILD_STATUS_FILE
    echo "JDK - $JDK" >> $BUILD_STATUS_FILE
    echo "Qt - $QT_VERSION_STRING" >> $BUILD_STATUS_FILE
}

clone() {
    echo "#######CLONE#######"
    mkdir -p ${STAGING_AREA}
    pushd ${STAGING_AREA}
    [ -d 'libserialport' ]	|| git clone --recursive ${LIBSERIALPORT%|*} -b ${LIBSERIALPORT#*|} libserialport
    [ -d 'libiio' ]		|| git clone --recursive ${LIBIIO%|*}       -b ${LIBIIO#*|} libiio
    [ -d 'libad9361' ]  	|| git clone --recursive ${LIBAD9361%|*}    -b ${LIBAD9361#*|} libad9361
    [ -d 'libm2k' ]	    	|| git clone --recursive ${LIBM2K%|*}       -b ${LIBM2K#*|} libm2k
    [ -d 'spdlog' ]	    	|| git clone --recursive ${SPDLOG%|*}       -b ${SPDLOG#*|} spdlog
    [ -d 'gr-scopy' ]   	|| git clone --recursive ${GRSCOPY%|*}      -b ${GRSCOPY#*|} gr-scopy
    [ -d 'gr-m2k' ]	    	|| git clone --recursive ${GRM2K%|*}        -b ${GRM2K#*|} gr-m2k
    [ -d 'volk' ]	    	|| git clone --recursive ${VOLK%|*}         -b ${VOLK#*|} volk
    [ -d 'gnuradio' ]   	|| git clone --recursive ${GNURADIO%|*}     -b ${GNURADIO#*|} gnuradio
    [ -d 'qwt' ]		|| git clone --recursive ${QWT%|*}          -b ${QWT#*|} qwt
    [ -d 'libsigrokdecode' ] 	|| git clone --recursive ${LIBSIGROKDECODE%|*} -b ${LIBSIGROKDECODE#*|} libsigrokdecode
    [ -d 'libtinyiiod' ] 	|| git clone --recursive ${LIBTINYIIOD%|*}  -b ${LIBTINYIIOD#*|} libtinyiiod
    [ -d 'KDDockWidgets' ] 	|| git clone --recursive ${KDDOCK%|*}       -b ${KDDOCK#*|} KDDockWidgets
    [ -d 'libsndfile' ]		|| git clone --recursive ${LIBSNDFILE%|*}   -b ${LIBSNDFILE#*|} libsndfile
    [ -d 'python' ]		|| git clone --recursive ${PYTHON%|*}       -b ${PYTHON#*|} python
    [ -d 'glib' ]		|| git clone --recursive ${GLIB%|*}         -b ${GLIB#*|} glib
    [ -d 'boost-for-android' ] 	|| git clone --recursive ${ANDROIDBOOST%|*} -b ${ANDROIDBOOST#*|} boost-for-android
    [ -d 'libxml2' ]		|| git clone --recursive ${LIBXML2%|*}      -b ${LIBXML2#*|} libxml2
    [ -d 'libiconv' ] 		|| git clone --recursive ${LIBICONV%|*}     -b ${LIBICONV#*|} libiconv
    [ -d 'gettext' ] 		|| git clone             ${GETTEXT%|*}      -b ${GETTEXT#*|} gettext
    [ -d 'fftw' ] 		|| git clone --recursive ${FFTW%|*}         -b ${FFTW#*|} fftw
    [ -d 'libgmp' ] 		|| git clone --recursive ${LIBGMP%|*}       -b ${LIBGMP#*|} libgmp
    [ -d 'libusb' ] 		|| git clone --recursive ${LIBUSB%|*}       -b ${LIBUSB#*|} libusb
    [ -d 'libzmq' ] 		|| git clone --recursive ${LIBZMQ%|*}       -b ${LIBZMQ#*|} libzmq
    [ -d 'libffi' ] 		|| git clone --recursive ${LIBFFI%|*}       -b ${LIBFFI#*|} libffi
    [ -d 'gnulib' ] 		|| git clone --recursive ${GNULIB%|*}       -b ${GNULIB#*|} gnulib
    [ -d 'libmatio' ]		|| git clone             ${LIBMATIO%|*}     -b ${LIBMATIO#*|} libmatio
    [ -d 'extra-cmake-modules' ] || git clone --recursive ${ECM%|*}         -b ${ECM#*|} extra-cmake-modules
    [ -d 'karchive' ] 		|| git clone --recursive ${KARCHIVE%|*}     -b ${KARCHIVE#*|} karchive
    [ -d 'libzstd' ] 		|| git clone --recursive ${LIBZSTD%|*}     -b ${LIBZSTD#*|} libzstd
    [ -d 'iio-emu' ] 		|| git clone --recursive ${IIOEMU%|*}       -b ${IIOEMU#*|} iio-emu

#     git clone https://github.com/openssl/openssl/ -b OpenSSL_1_1_1w


    popd
}

build_with_cmake() {
    INSTALL=$1
    [ -z $INSTALL ] && INSTALL=OFF
    BUILD_FOLDER=$PWD/build
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && rm -rf $BUILD_FOLDER
    mkdir -p $BUILD_FOLDER
    cd $BUILD_FOLDER
    set_cmake_opts
    $CMAKE $CURRENT_BUILD_CMAKE_OPTS  -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS -landroid" -DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS" -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CPPFLAGS -g -O0" ../
    make $JOBS
    if [ "$INSTALL" == "ON" ];then
        strip
        make install
    fi
    CURRENT_BUILD_CMAKE_OPTS=""
    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE
}

build_with_configure() {
    INSTALL=$1
    [ -z $INSTALL ] && INSTALL=ON
    [ -f "./autogen.sh" ] && NOCONFIGURE=yes ./autogen.sh
    set_configure_opts
    ./configure "${CONFIG_OPTS[@]}" "${@:2}"
    #LD_RUN_PATH=$STAGING_AREA_DEPS/lib
    make $JOBS
    strip
    if [ "$INSTALL" == "ON" ];then
        strip
        make install
    fi
    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE
}

build_libiconv() {
    pushd ${STAGING_AREA}/libiconv
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    export GNULIB_SRCDIR=${STAGING_AREA}/gnulib
    build_with_configure $1 --enable-static=no --enable-shared=yes
    popd
}

build_openssl() {
    pushd ${STAGING_AREA}/openssl
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    export ANDROID_NDK_HOME=${ANDROID_NDK_ROOT}
    export PATH=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH

    ./Configure android-arm64 shared no-ssl3 no-comp --prefix=${STAGING_AREA_DEPS}
    make $JOBS
  #  make install
#     ./Configure android-arm -D__ARM_MAX_ARCH__=7 --prefix=${PREFIX} shared no-ssl3 no-comp
    popd
}

build_libffi() {
    pushd ${STAGING_AREA}/libffi
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    build_with_configure $1 --disable-docs --cache-file=android.cache --disable-multi-os-directory
    popd
}

build_gettext() {
    pushd ${STAGING_AREA}/gettext
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    # rm -rf ./autogen.sh # hack

    LDFLAGS_OLD=${LDFLAGS}
    export LDFLAGS="${LDFLAGS} -L${STAGING_AREA}/gettext/gettext-tools/gnulib-lib/.libs"

    export GNULIB_SRCDIR=${STAGING_AREA}/gnulib
    build_with_configure $1 --cache-file=android.cache

    export LDFLAGS=${LDFLAGS_OLD}
    popd
}

build_glib() {
    INSTALL=$1
    pushd ${STAGING_AREA}/glib
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    export CURRENT_BUILD=glib
    echo "
    [host_machine]
    system = 'android'
    cpu_family = 'aarch64'
    cpu = 'aarch64'
    endian = 'little'

    [properties]
    pkg_config_libdir = '$STAGING_AREA_DEPS/lib/pkgconfig'
    sys_root = '$SYSROOT'
    so_version = ''

    [binaries]
    c = '$CC'
    cpp = '$CPP'
    cxx = '$CXX'
    ar = '$AR'
    as = '$AS'
    ld = '$LD'
    nm = '$NM'
    strip = '$STRIP'

    [built-in options]
    c_std = 'c17'
    prefix = '$STAGING_AREA_DEPS'
    c_args = [$(meson_flag_list $CFLAGS)]
    cpp_args = [$(meson_flag_list $f)]
    c_link_args = [$(meson_flag_list $LDFLAGS)]
    pkg_config_path = '$STAGING_AREA_DEPS/lib/pkgconfig'
    default_library = 'shared'
    " > cross_file.txt

    pip install meson
    mkdir -p ${STAGING_AREA}/glib/build
    ~/.local/bin/meson setup --cross-file cross_file.txt build
    pushd ${STAGING_AREA}/glib/build
    ~/.local/bin/meson compile

    if [ "$INSTALL" == "ON" ];then
        strip
        ~/.local/bin/meson install
    fi

    popd
    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE
    popd
}

build_libxml2() {
    pushd ${STAGING_AREA}/libxml2
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    CURRENT_BUILD_CMAKE_OPTS="\
        -DLIBXML2_WITH_LZMA=OFF \
        -DLIBXML2_WITH_PYTHON=OFF \
        -DLIBXML2_WITH_TESTS=OFF \
        -DLIBXML2_WITH_ZLIB=OFF
    "
    build_with_cmake $1
    popd
}

build_boost() {
    pushd ${STAGING_AREA}/boost-for-android
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    ./build-android.sh --boost=1.82.0 --layout=system --toolchain=llvm --prefix=${STAGING_AREA_DEPS} --arch=$ABI --target-version=${API} ${ANDROID_NDK_ROOT}

    cp -R $STAGING_AREA_DEPS/$ABI/* $STAGING_AREA_DEPS
    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE
    popd
}

build_libzmq() {
    pushd ${STAGING_AREA}/libzmq
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    CURRENT_BUILD_CMAKE_OPTS=""
    build_with_cmake $1

    popd
}

build_fftw() {
    pushd ${STAGING_AREA}/fftw

    ## ADI COMMENT: USE downloaded version instead (OCAML fail?)
    # wget http://www.fftw.org/fftw-3.3.9.tar.gz
    # rm -rf fftw-3.3.9
    # tar xvf fftw-3.3.9.tar.gz

    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    touch ChangeLog
    rm -rf autom4te.cache
    autoreconf --verbose --install --symlink --force
    rm -rf config.cache

    build_with_configure $1 \
        --enable-shared --enable-threads --enable-single \
        --enable-float --enable-neon --disable-doc \
        --disable-shared --enable-maintainer-mode

    popd
}

build_libgmp() {
    pushd ${STAGING_AREA}/libgmp
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    ABI=64 ./.bootstrap
    ABI=64 build_with_configure $1 --enable-maintainer-mode --enable-cxx
    popd
}

build_libusb() {
    INSTALL=$1
    pushd ${STAGING_AREA}/libusb/android/jni
    # WE NEED TO USE BetterAndroidSupport PR from libusb
    # this will be merged to mainline soon
    # https://github.com/libusb/libusb/pull/874

	rm -v Application.mk
	echo "
	APP_ABI := arm64-v8a
	APP_PLATFORM := android-31

	APP_CFLAGS := \
	-g \
	-std=gnu11 \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wunused \
	-Wwrite-strings \
	-Werror=format-security \
	-Werror=implicit-function-declaration \
	-Werror=implicit-int \
	-Werror=init-self \
	-Werror=missing-prototypes \
	-Werror=strict-prototypes \
	-Werror=undef \
	-Werror=uninitialized

	APP_LDFLAGS := -llog
	" > Application.mk

    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    export NDK=${ANDROID_NDK_ROOT}
    ${NDK}/ndk-build clean
    ${NDK}/ndk-build -B -r -R -E "APP_PLATFORM=android-$API"

    if [ "$INSTALL" == "ON" ];then
        strip
        echo "INSTALLING libusb"
        cp -v ${STAGING_AREA}/libusb/android/libs/$ABI/* ${STAGING_AREA_DEPS}/lib
        cp -v  ${STAGING_AREA_DEPS}/lib/libusb1.0.so $STAGING_AREA_DEPS/lib/libusb-1.0.so # IDK why this happens (?)
        cp -v ${STAGING_AREA}/libusb/libusb/libusb.h ${STAGING_AREA_DEPS}/include
    fi

    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE
    popd
}

build_libserialport(){
    echo "### Building libserialport - branch $LIBSERIALPORT_BRANCH"
    pushd $STAGING_AREA/libserialport
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    build_with_configure $1
    popd
}

# momentatn HAVE_DNS_SD=OFF
build_libiio() {
    echo "### Building libiio - version $LIBIIO_VERSION"
    pushd ${STAGING_AREA}/libiio
    CURRENT_BUILD_CMAKE_OPTS="\
        -DWITH_TESTS:BOOL=OFF \
        -DWITH_DOC:BOOL=OFF \
        -DHAVE_DNS_SD:BOOL=OFF \
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
    pushd ${STAGING_AREA}/libad9361
    build_with_cmake $1
    popd
}

build_spdlog() {
    echo "### Building spdlog - branch $SPDLOG_BRANCH"
    pushd ${STAGING_AREA}/spdlog
    CURRENT_BUILD_CMAKE_OPTS="-DSPDLOG_BUILD_SHARED=ON"
    build_with_cmake $1
    popd
}

build_libm2k() {
    echo "### Building libm2k - branch $LIBM2K_BRANCH"
    pushd ${STAGING_AREA}/libm2k
    CURRENT_BUILD_CMAKE_OPTS="\
        -DENABLE_PYTHON=OFF \
        -DENABLE_CSHARP=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DENABLE_TOOLS=ON \
        -DINSTALL_UDEV_RULES=OFF \
        "
    build_with_cmake $1
    popd
}

build_volk() {
    echo "### Building volk - branch $VOLK_BRANCH"
    pushd ${STAGING_AREA}/volk
    CURRENT_BUILD_CMAKE_OPTS="\
        -DPYTHON_EXECUTABLE=/usr/bin/python3 \
        -DVOLK_CPU_FEATURES=ON \
        -DBOOST_ROOT=${STAGING_AREA_DEPS} \
        -DBoost_COMPILER=-clang \
        -DBoost_USE_STATIC_LIBS=ON \
        -DBoost_ARCHITECTURE=-a32 \
        -DENABLE_STATIC_LIBS=False \
        "
    build_with_cmake $1
    popd
}

build_libsndfile() {
    echo "### Building libsndfile - branch $SNDFILE_BRANCH"
    pushd ${STAGING_AREA}/libsndfile
    CURRENT_BUILD_CMAKE_OPTS="-DWITH_PYTHON=OFF"
    build_with_cmake $1
    popd
}

build_gnuradio() {
    echo "### Building gnuradio - branch $GNURADIO_BRANCH"
    pushd ${STAGING_AREA}/gnuradio

    CURRENT_BUILD_CMAKE_OPTS="\
        -DPYTHON_EXECUTABLE=/usr/bin/python3 \
        -DENABLE_DEFAULT=OFF \
        -DENABLE_INTERNAL_VOLK=OFF \
        -DENABLE_GNURADIO_RUNTIME=ON \
        -DENABLE_GR_ANALOG=ON \
        -DENABLE_GR_BLOCKS=ON \
        -DENABLE_GR_FFT=ON \
        -DENABLE_GR_FILTER=ON \
        -DENABLE_GR_IIO=ON \
        -DENABLE_POSTINSTALL=OFF \
        -DBOOST_ROOT=${STAGING_AREA_DEPS} \
        -DBoost_COMPILER=-clang \
        -DBoost_USE_STATIC_LIBS=ON \
        -DBoost_ARCHITECTURE=-a64 \
        -DENABLE_DOXYGEN=OFF \
        "
    build_with_cmake $1
    popd
}

build_grscopy() {
    echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
    pushd ${STAGING_AREA}/gr-scopy
    CURRENT_BUILD_CMAKE_OPTS="-DWITH_PYTHON=OFF"
    build_with_cmake $1
    popd
}

build_grm2k() {
    echo "### Building gr-m2k - branch $GRM2K_BRANCH"
    pushd ${STAGING_AREA}/gr-m2k
        CURRENT_BUILD_CMAKE_OPTS="\
        -DENABLE_PYTHON=OFF \
        -DDIGITAL=OFF
        "
    build_with_cmake $1
    popd
}

build_qwt() {
    echo "### Building qwt - branch $QWT_BRANCH"
    pushd ${STAGING_AREA}/qwt
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    INSTALL=$1
    [ -z $INSTALL ] && INSTALL=ON

    $QMAKE \
    	ANDROID_ABIS=$ABI \
    	ANDROID_MIN_SDK_VERSION=$API \
	ANDROID_API_VERSION=$API \
	INCLUDEPATH+=$STAGING_AREA_DEPS/include \
	LIBS+=-L$STAGING_AREA_DEPS/lib \
	qwt.pro

    make $JOBS

    if [ "$INSTALL" == "ON" ];then
        strip
        make INSTALL_ROOT=$STAGING_AREA_DEPS install
    fi

    cp -R $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
    cp -R $STAGING_AREA_DEPS/libs/$ABI/* $STAGING_AREA_DEPS/lib # another hack
    cp -R $QT_LOCATION/lib/libQt5PrintSupport*.so $STAGING_AREA_DEPS/lib

    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE

    popd
}

build_python() {
    pushd ${STAGING_AREA}/python
    # Python should be cross-built with the same version that is available on host, if nothing is available,
    # it should be built with the script ./build_host_python

    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf

    export ac_cv_lib_intl_textdomain=no
    export ac_cv_header_libintl_h=no
    export ac_cv_func_bind_textdomain_codeset=no
    export ac_cv_func_dcgettext=no
    export ac_cv_func_gettext=no
    export ac_cv_func_dgettext=no
    export ac_cv_file__dev_ptmx=no
    export ac_cv_file__dev_ptc=no
    export ac_cv_func_pipe2=no
    export ac_cv_func_fdatasync=no
    export ac_cv_func_killpg=no
    export ac_cv_func_waitid=no
    export ac_cv_func_sigaltstack=no
    export ac_cv_func_bind_textdomain_codeset=no
    export ac_cv_lib_intl_textdomain=no

    autoreconf
    build_with_configure $1 \
	--disable-ipv6 \
	--with-build-python \
	--without-ensurepip \
	--without-c-locale-coercion \
	--without-doc-strings \

    sed -i "s/^#zlib/zlib/g" Modules/Setup
    # 11
    sed -i "s/^#math/math/g" Modules/Setup
    sed -i "s/^#time/time/g" Modules/Setup
    sed -i "s/^#_struct/_struct/g" Modules/Setup

    make $JOBS LDFLAGS="$LDFLAGS -liconv -lz -lm"
    make install
    rm -rf $STAGING_AREA_DEPS/lib/python3.11/test

    echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
    >> $BUILD_STATUS_FILE
    popd
}

build_libsigrokdecode() {
    echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
    pushd ${STAGING_AREA}/libsigrokdecode
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    build_with_configure $1
    popd
}

build_libtinyiiod() {
    echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
    pushd ${STAGING_AREA}/libtinyiiod
    CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
    build_with_cmake $1
    popd
}

build_kddock() {
    echo "### Building KDDockWidgets - branch $KDDOCK_BRANCH"
    pushd ${STAGING_AREA}/KDDockWidgets
    CURRENT_BUILD_CMAKE_OPTS=""
    build_with_cmake $1
    popd
}

build_libmatio() {
    echo "### Building libmatio - branch $LIBMATIO_BRANCH"
    pushd $STAGING_AREA/libmatio
    CURRENT_BUILD_CMAKE_OPTS="-DMATIO_MAT73=OFF -DMATIO_SHARED=ON -DMATIO_WITH_HDF5=OFF" # nu sunt sigur ce sa fie trebuie disabled
    build_with_cmake $1
    popd
}

build_libzstd() {
    pushd $STAGING_AREA/libzstd/build/cmake
    CURRENT_BUILD_CMAKE_OPTS="-DZSTD_BUILD_SHARED=ON -DZSTD_BUILD_STATIC=OFF=, "
    build_with_cmake $1
    popd
}

build_ecm() {
	echo "### Building extra-cmake-modules (ECM) - branch $ECM_BRANCH"
	pushd $STAGING_AREA/extra-cmake-modules
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DBUILD_QTHELP_DOCS=OFF"
	build_with_cmake $1
	popd
}

build_karchive () {
	echo "### Building karchive - version $KARCHIVE_BRANCH"
	pushd $STAGING_AREA/karchive
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DBUILD_TESTING=OFF"
	build_with_cmake $1
	popd
}

build_iio-emu() {
    echo "### Building iio-emu - branch $IIOEMU_BRANCH"
    mkdir -p ${STAGING_AREA}
    pushd ${STAGING_AREA}/iio-emu
    build_with_cmake OFF
    popd
}


build_scopy() {
    pushd ${SRC_DIR}
    CURRENT_BUILD_CMAKE_OPTS="\
        -DANDROID_STAGING_LOCATION:FILEPATH=${STAGING_AREA_DEPS}/lib \
        -DANDROID_DEPLOY_QT:FILEPATH=$QT_LOCATION/bin/androiddeployqt \
        -DANDROID_PACKAGE_SOURCE_DIR:FILEPATH=$SRC_DIR/android \
        -DANDROID_VERSION_CODE=0104010000\
        -DANDROID_VERSION_NAME=2.2.0 \
	-DANDROID_MIN_SDK_VERSION=${API} \
	-DANDROID_TARGET_SDK_VERSION=${API}
	"

#   -DQT_ANDROID_DEPLOYMENT_DEPENDENCIES=
#   -DQT_ANDROID_EXTRA_PLUGINS=

    build_with_cmake OFF
    popd
}


copy_deps()
{
	STAGING_DEPSS=/home/cristian/git/android/scopy-android/ci/android/staging/dependencies
	BUILDD=/home/cristian/git/android/scopy-android/build
	ASSETSSS=/home/cristian/git/android/scopy-android/android/assets

	DEPS=(
		$QT_LOCATION/lib/libQt5PrintSupport_arm64-v8a.so
		$STAGING_DEPSS/lib/libad9361.so
		$STAGING_DEPSS/lib/libcharset.so
		$STAGING_DEPSS/lib/libffi.so
		$STAGING_DEPSS/lib/libgio-2.0.so
		$STAGING_DEPSS/lib/libglib-2.0.so
		$STAGING_DEPSS/lib/libgmodule-2.0.so
		$STAGING_DEPSS/lib/libgmp.so
		$STAGING_DEPSS/lib/libgmpxx.so
		$STAGING_DEPSS/lib/libgnuradio-analog.so
		$STAGING_DEPSS/lib/libgnuradio-blocks.so
		$STAGING_DEPSS/lib/libgnuradio-fft.so
		$STAGING_DEPSS/lib/libgnuradio-filter.so
		$STAGING_DEPSS/lib/libgnuradio-iio.so
		$STAGING_DEPSS/lib/libgnuradio-m2k.so
		$STAGING_DEPSS/lib/libgnuradio-pmt.so
		$STAGING_DEPSS/lib/libgnuradio-runtime.so
		$STAGING_DEPSS/lib/libgnuradio-scopy.so
		$STAGING_DEPSS/lib/libgobject-2.0.so
		$STAGING_DEPSS/lib/libgthread-2.0.so
		$STAGING_DEPSS/lib/libiconv.so
		$STAGING_DEPSS/lib/libiio.so
		$STAGING_DEPSS/lib/libintl.so
		$STAGING_DEPSS/lib/libirmp.so
		$STAGING_DEPSS/lib/libm2k.so
		$STAGING_DEPSS/lib/libqwt_arm64-v8a.so
		$STAGING_DEPSS/lib/libserialport.so
		$STAGING_DEPSS/lib/libsigrokdecode.so
		$STAGING_DEPSS/lib/libspdlogd.so
		$STAGING_DEPSS/lib/libusb-1.0.so
		$STAGING_DEPSS/lib/libusb1.0.so
		$STAGING_DEPSS/lib/libvolk.so
		$STAGING_DEPSS/lib/libxml2.so
		$STAGING_DEPSS/lib/libzmq.so
		$STAGING_DEPSS/lib/tinyiiod.so
		$STAGING_DEPSS/lib/libzstd.so
		$STAGING_DEPSS/lib/libKF5Archive_arm64-v8a.so
		$STAGING_DEPSS/lib/libmatio.so


		/mnt/ssd/Qt5-kde-android-arm64/lib/libQt5OpenGL_arm64-v8a.so

	)
	# $STAGING_DEPSS/lib/preloadable_libiconv.so
	# $STAGING_DEPSS/lib/libtextstyle.so

	cp -vfr ${DEPS[@]} $BUILDD/android-build/libs/arm64-v8a/
	# a se copia cu extensia .so ca sa pastrezi flagu de executable
	cp -vf /home/cristian/git/android/scopy-android/ci/android/staging/iio-emu/build/iio-emu $BUILDD/android-build/libs/arm64-v8a/iio-emu.so

	cp -vfr $BUILDD/libscopy* $BUILDD/android-build/libs/arm64-v8a/
	cp -vfr $BUILDD/style $ASSETSSS
	cp -vfr $STAGING_DEPSS/lib/scopy/packages $ASSETSSS
	cp -vfr $BUILDD/translations $ASSETSSS
	cp -vfr /home/cristian/git/android/scopy-android/ci/android/staging/dependencies/share/libsigrokdecode/decoders $ASSETSSS
	[ -f $ASSETSSS/python3.11 ] || cp -vfr /home/cristian/git/android/scopy-android/ci/android/staging/dependencies/lib/python3.11 $ASSETSSS
	echo "Copied dependencies to assets"

}

# "
# "Cannot load library assets:/scopy-plugins/libscopy-m2k_arm64-v8a.so
# err: Cannot load library assets:/scopy-plugins/libscopy-m2k_arm64-v8a.so:
# (dlopen failed: library \"assets:_scopy-plugins_libscopy-m2k_arm64-v8a.so\" not found)"
# "


build_deps()
{
	echo "buildd"
# #     create_build_status_file
# #     build_openssl ON #  de vazut daca trebe ???
#     build_libiconv ON
#     build_libffi ON
#    # build_gettext ON # disabled for now, gnulib error (oprit de tot ca nu mai e nevoie de el la python)
#     build_libiconv ON # HANDLE CIRCULAR DEP
#     build_glib ON #### libglib2.0.so.0 ---> libglib2.0.so
#     build_libxml2 ON
#     build_boost
#     build_libzmq ON
#     build_fftw ON
#     build_libgmp ON
     build_libusb ON
#     build_libserialport ON
#     build_libsndfile ON
     build_libiio ON
#     build_libad9361 ON
#     build_spdlog ON
#     build_libm2k ON
#     build_volk ON
#     build_gnuradio ON # am facut modificari in cod si trebuie puse sus
#     build_grscopy ON
#     build_grm2k ON
#     build_qwt ON # am facut modificari in cod si trebuie puse sus
#     build_python ON # disable locale, nu mai e nevoie de gettext, update la python 3.11
#     build_libsigrokdecode ON
#     build_libtinyiiod ON
#     build_libmatio ON
#       build_libzstd ON
#       build_ecm ON
#       build_karchive ON

}

for arg in "${@}"; do
    $arg
done



###

# build_openssl() {
#     pushd ${STAGING_AREA}/openssl
#     [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
#     export CURRENT_BUILD=openssl

#     export ANDROID_NDK_HOME=${ANDROID_NDK_ROOT}

#     ./Configure android-arm -D__ARM_MAX_ARCH__=7 --prefix=${PREFIX} shared no-ssl3 no-comp
#     make ${JOBS}
#     make install
#     popd
# }

build_thrift() {
    pushd ${STAGING_AREA}/thrift
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    export CURRENT_BUILD=thrift
    rm -rf ${PREFIX}/include/thrift

    ./bootstrap.sh

    CPPFLAGS="-I${PREFIX}/include" \
    CFLAGS="-fPIC" \
    CXXFLAGS="-fPIC" \
    LDFLAGS="-L${PREFIX}/lib" \
    ./configure --prefix=${PREFIX}   --disable-tests --disable-tutorial --with-cpp \
    --without-python --without-qt4 --without-qt5 --without-py3 --without-go --without-nodejs --without-c_glib --without-php --without-csharp --without-java \
    --without-libevent --without-zlib \
    --with-boost=${PREFIX} --host=$TARGET_BINUTILS --build=x86_64-linux

    sed -i '/malloc rpl_malloc/d' ./lib/cpp/src/thrift/config.h
    sed -i '/realloc rpl_realloc/d' ./lib/cpp/src/thrift/config.h

    make ${JOBS}
    make install

    sed -i '/malloc rpl_malloc/d' ${PREFIX}/include/thrift/config.h
    sed -i '/realloc rpl_realloc/d' ${PREFIX}/include/thrift/config.h
    popd
}

build_gr-grand() {
    pushd ${STAGING_AREA}/gr-grand
    [ ${CLEAN_BEFORE_BUILD} == "ON" ] && git clean -xdf
    export CURRENT_BUILD=gr-grand

    mkdir build
    cd build

    $CMAKE -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ABI -DANDROID_ARM_NEON=ON \
    -DANDROID_NATIVE_API_LEVEL=${API} \
    -DANDROID_STL=c++_shared \
    -DBOOST_ROOT=${PREFIX} \
    -DBoost_COMPILER=-clang \
    -DBoost_USE_STATIC_LIBS=ON \
    -DBoost_ARCHITECTURE=-a32 \
    -DGnuradio_DIR=${STAGING_AREA}/toolchain/$ABI/lib/cmake/gnuradio \
    -DCMAKE_FIND_ROOT_PATH=${PREFIX} \
        ../

    make ${JOBS}
    make install
    popd
}

###

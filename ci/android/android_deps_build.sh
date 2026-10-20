#!/bin/bash -xe

# Scopy Android (Qt6) dependency build
# ====================================
# Usage: ABI=x86_64 ./android_deps_build.sh build_deps
#        ABI=arm64-v8a ./android_deps_build.sh clone build_deps verify_deps
#
# Builds only what the slim Scopy build actually links. M2K is force-disabled in
# packages/CMakeLists.txt and ENABLE_PLUGIN_ADC defaults OFF (which skips
# gr-util), so gnuradio, volk, genalyzer, boost, matio, libsigrokdecode, glib and
# python are all out of scope. libserialport IS built: iioscantask.cpp includes
# libserialport.h and calls sp_* unconditionally, so Scopy cannot build without it.

SRC_SCRIPT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SRC_SCRIPT/android_toolchain.sh

# Per-dependency make variable overrides, reset by each build_with_* helper.
CURRENT_BUILD_MAKE_ARGS=()

build_with_cmake() {
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	set_cmake_opts
	BUILD_FOLDER=$PWD/build-$ABI
	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	pushd $BUILD_FOLDER
	# "${CMAKE_OPTS[@]}" must stay quoted: flattening the array into a string and
	# re-splitting it unquoted breaks every multi-word *_FLAGS value.
	$CMAKE_BIN "${CMAKE_OPTS[@]}" $CURRENT_BUILD_CMAKE_OPTS ../
	make $JOBS
	[ "$INSTALL" == "ON" ] && make install
	popd
	CURRENT_BUILD_CMAKE_OPTS=""
}

build_with_configure() {
	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	set_configure_opts
	./autogen.sh
	./configure "${CONFIG_OPTS[@]}" $CURRENT_BUILD_CONFIG_OPTS
	make $JOBS "${CURRENT_BUILD_MAKE_ARGS[@]}"
	[ "$INSTALL" == "ON" ] && make install "${CURRENT_BUILD_MAKE_ARGS[@]}"
	CURRENT_BUILD_CONFIG_OPTS=""
	CURRENT_BUILD_MAKE_ARGS=()
}

# The Android SDK: cmdline-tools (for sdkmanager), the NDK, and exactly one platform and one
# build-tools revision - see android_toolchain.sh for why "exactly one".
#
# Nothing newer than android-$ANDROID_SDK_PLATFORM can turn up, because sdkmanager installs only
# the packages it is named. That makes the symlink-farm workaround check_sdk_platforms describes
# unnecessary in a purpose-built image, and leaves that guard as a cheap regression check.
#
# Deliberately NOT installed: emulator, system-images, sources and the SDK's own cmake - several
# GB that nothing in this build touches.
#
# Idempotent, so configure_system can re-enter it and the Dockerfile can keep it in its own layer.
download_sdk() {
	echo "### Installing Android SDK - platform $ANDROID_SDK_PLATFORM, build-tools $ANDROID_SDK_BUILD_TOOLS, NDK $NDK_VERSION"
	if [ ! -x "$SDKMANAGER" ]; then
		local dl=$STAGING_AREA/cmdline-tools-download
		rm -rf $dl
		mkdir -p $dl
		pushd $dl
		wget -q https://dl.google.com/android/repository/commandlinetools-linux-${ANDROID_CMDLINE_TOOLS}_latest.zip -O cmdline-tools.zip
		unzip -q cmdline-tools.zip
		# The zip unpacks to ./cmdline-tools, but sdkmanager only infers its SDK root when it
		# sits at $ANDROID_SDK_ROOT/cmdline-tools/latest, so install it there.
		mkdir -p $ANDROID_SDK_ROOT/cmdline-tools
		mv cmdline-tools $ANDROID_SDK_ROOT/cmdline-tools/latest
		popd
		rm -rf $dl
	fi

	# Licences have to be accepted before anything will install; stdout is thousands of lines of
	# licence text.
	yes | $SDKMANAGER --sdk_root=$ANDROID_SDK_ROOT --licenses > /dev/null

	$SDKMANAGER --sdk_root=$ANDROID_SDK_ROOT \
		"platform-tools" \
		"platforms;android-$ANDROID_SDK_PLATFORM" \
		"build-tools;$ANDROID_SDK_BUILD_TOOLS" \
		"ndk;$NDK_VERSION"

	check_sdk_platforms
}

clone() {
	mkdir -p $STAGING_AREA_SRC
	pushd $STAGING_AREA_SRC
	[ -d 'libserialport' ]	|| git clone --recursive https://github.com/sigrokproject/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	[ -d 'libxml2' ]	|| git clone --recursive https://github.com/GNOME/libxml2.git -b $LIBXML2_BRANCH libxml2
	[ -d 'libiio' ]		|| git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	[ -d 'libad9361' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'extra-cmake-modules' ] || git clone --recursive https://github.com/KDE/extra-cmake-modules.git -b $ECM_BRANCH extra-cmake-modules
	[ -d 'karchive' ]	|| git clone --recursive https://github.com/KDE/karchive.git -b $KARCHIVE_BRANCH karchive
	[ -d 'qcoro' ]		|| git clone --recursive https://github.com/qcoro/qcoro.git -b $QCORO_BRANCH qcoro
	[ -d 'KDDockWidgets' ]	|| git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	popd
}

build_libserialport() {
	echo "### Building libserialport - $LIBSERIALPORT_BRANCH"
	pushd $STAGING_AREA_SRC/libserialport
	git clean -xdf
	# Android will not load a versioned libfoo.so.N, and androiddeployqt rejects any
	# extra library whose name is not exactly lib*.so. libserialport's Makefile.am
	# sets "-version-info $(SP_LIB_VERSION) -no-undefined", so override that variable
	# at make time to swap in -avoid-version.
	#
	# It has to be done this way: -avoid-version is a libtool flag, and putting it in
	# LDFLAGS breaks configure before anything is built - configure passes LDFLAGS
	# straight to the compiler for its "C compiler cannot create executables" check,
	# and clang errors with "unknown argument: '-avoid-version'".
	CURRENT_BUILD_MAKE_ARGS=("libserialport_la_LDFLAGS=-avoid-version -no-undefined")
	build_with_configure $1
	popd
}

build_libxml2() {
	echo "### Building libxml2 - $LIBXML2_BRANCH"
	pushd $STAGING_AREA_SRC/libxml2
	# iconv is dropped (no Android build of it in this dep set) and the optional
	# compressors are not used by libiio's XML reader.
	CURRENT_BUILD_CMAKE_OPTS="\
		-DBUILD_SHARED_LIBS=ON \
		-DLIBXML2_WITH_ICONV=OFF \
		-DLIBXML2_WITH_PYTHON=OFF \
		-DLIBXML2_WITH_ZLIB=OFF \
		-DLIBXML2_WITH_LZMA=OFF \
		-DLIBXML2_WITH_TESTS=OFF \
		-DLIBXML2_WITH_PROGRAMS=OFF \
		"
	build_with_cmake $1
	popd
}

build_libiio() {
	echo "### Building libiio - $LIBIIO_VERSION"
	pushd $STAGING_AREA_SRC/libiio
	# The USB backend needs libusb plus the Android UsbManager file-descriptor
	# handoff (libusb_wrap_sys_device), which is a follow-up milestone. The serial
	# backend is on, matching every other platform.
	CURRENT_BUILD_CMAKE_OPTS="\
		-DWITH_TESTS=OFF \
		-DWITH_DOC=OFF \
		-DWITH_MATLAB_BINDINGS=OFF \
		-DCSHARP_BINDINGS=OFF \
		-DPYTHON_BINDINGS=OFF \
		-DHAVE_DNS_SD=OFF \
		-DENABLE_IPV6=OFF \
		-DINSTALL_UDEV_RULE=OFF \
		-DWITH_SERIAL_BACKEND=ON \
		-DWITH_USB_BACKEND=OFF \
		-DWITH_LOCAL_BACKEND=OFF \
		-DWITH_NETWORK_BACKEND=ON \
		"
	build_with_cmake $1
	popd
}

build_libad9361() {
	echo "### Building libad9361 - $LIBAD9361_BRANCH"
	pushd $STAGING_AREA_SRC/libad9361
	build_with_cmake $1
	popd
}

build_qwt() {
	echo "### Building qwt - $QWT_BRANCH (as qwt_scopy_$ABI)"
	pushd $STAGING_AREA_SRC/qwt
	git clean -xdf
	# gui/CMakeLists.txt looks for an ABI-suffixed name on Android
	# (find_library(... NAMES qwt_scopy_${ANDROID_ABI})), because all ABIs' libs
	# end up in one flat dir. qmake does not add that suffix itself - unlike
	# qt_add_library, which appends _<abi> via the SUFFIX property - so bake it
	# into the target name.
	sed -i "s|qwtLibraryTarget(qwt)|qwtLibraryTarget(qwt_scopy_$ABI)|" src/src.pro
	sed -i "s|QWT_SONAME=libqwt.so|QWT_SONAME=libqwt_scopy_$ABI.so|" src/src.pro
	# Subprojects we do not ship; they link qwt by its old name and would fail
	# with "cannot find -lqwt".
	sed -i 's|^QWT_CONFIG     += QwtDesigner|#QWT_CONFIG     += QwtDesigner|' qwtconfig.pri
	sed -i 's|^QWT_CONFIG     += QwtExamples|#QWT_CONFIG     += QwtExamples|' qwtconfig.pri
	sed -i 's|^QWT_CONFIG     += QwtPlayground|#QWT_CONFIG     += QwtPlayground|' qwtconfig.pri
	sed -i 's|^QWT_CONFIG     += QwtTests|#QWT_CONFIG     += QwtTests|' qwtconfig.pri

	INSTALL=$1
	[ -z $INSTALL ] && INSTALL=ON
	$QMAKE_BIN qwt.pro
	make $JOBS
	if [ "$INSTALL" == "ON" ]; then
		# Qt's Android mkspec overrides target.path for libraries, forcing it to
		# $(INSTALL_ROOT)/libs/<abi>/ and ignoring qwt's QWT_INSTALL_LIBS. Without
		# INSTALL_ROOT that resolves to an absolute /libs and fails on permissions.
		make INSTALL_ROOT=$STAGING_AREA_DEPS install
		# Normalise that Android-specific layout into the lib/ dir every other dep
		# installs into, so Scopy's find_library has a single place to look.
		mkdir -p $STAGING_AREA_DEPS/lib
		mv $STAGING_AREA_DEPS/libs/$ABI/libqwt_scopy_$ABI.so $STAGING_AREA_DEPS/lib/
		# qwt's unix install prefix is /usr/local, so with INSTALL_ROOT set the
		# headers land in <staging>/usr/local/include. Flatten them up one level so
		# gui/CMakeLists.txt finds qwt_global.h on the normal include path.
		cp -r $STAGING_AREA_DEPS/usr/local/* $STAGING_AREA_DEPS/
		rm -rf $STAGING_AREA_DEPS/libs $STAGING_AREA_DEPS/usr
	fi
	popd
}

build_ecm() {
	echo "### Building extra-cmake-modules - $ECM_BRANCH"
	pushd $STAGING_AREA_SRC/extra-cmake-modules
	CURRENT_BUILD_CMAKE_OPTS="\
		-DBUILD_HTML_DOCS=OFF \
		-DBUILD_MAN_DOCS=OFF \
		-DBUILD_QTHELP_DOCS=OFF \
		"
	build_with_cmake $1
	popd
}

build_karchive() {
	echo "### Building karchive - $KARCHIVE_BRANCH"
	pushd $STAGING_AREA_SRC/karchive
	# Scopy only uses the zip reader (pkg-manager); every other compressor would
	# drag in another cross-build. zlib comes from the NDK sysroot.
	CURRENT_BUILD_CMAKE_OPTS="\
		-DWITH_BZIP2=OFF \
		-DWITH_LIBLZMA=OFF \
		-DWITH_OPENSSL=OFF \
		-DWITH_LIBZSTD=OFF \
		"
	build_with_cmake $1
	popd
}

build_qcoro() {
	echo "### Building qcoro - $QCORO_BRANCH"
	pushd $STAGING_AREA_SRC/qcoro
	CURRENT_BUILD_CMAKE_OPTS="\
		-DQCORO_BUILD_EXAMPLES=OFF \
		-DQCORO_BUILD_TESTING=OFF \
		-DQCORO_WITH_QTWEBSOCKETS=OFF \
		-DQCORO_WITH_QTQUICK=OFF \
		-DQCORO_WITH_QML=OFF \
		-DQCORO_WITH_QTDBUS=OFF \
		-DBUILD_SHARED_LIBS=ON \
		"
	build_with_cmake $1
	popd
}

build_kddock() {
	echo "### Building KDDockWidgets - $KDDOCK_BRANCH"
	pushd $STAGING_AREA_SRC/KDDockWidgets
	CURRENT_BUILD_CMAKE_OPTS="\
		-DKDDockWidgets_QT6=ON \
		-DKDDockWidgets_FRONTENDS=qtwidgets \
		-DKDDockWidgets_EXAMPLES=OFF \
		-DKDDockWidgets_TESTS=OFF \
		"
	build_with_cmake $1
	popd
}

build_deps() {
	print_config
	mkdir -p $STAGING_AREA_DEPS
	build_libserialport ON
	build_libxml2 ON
	build_libiio ON
	build_libad9361 ON
	build_qwt ON
	build_ecm ON
	build_karchive ON
	build_qcoro ON
	build_kddock ON
	verify_deps
}

# Android rejects versioned SONAMEs and anything not named lib*.so, and
# androiddeployqt silently SKIPS libraries of the wrong architecture rather than
# failing - so a bad dep shows up as a missing library at runtime, not a build
# error. Check both properties up front.
verify_deps() {
	echo "### Verifying $STAGING_AREA_DEPS/lib for ABI $ABI"
	local expected fail=0
	# llvm-readelf spells x86_64 as "Advanced Micro Devices X86-64", so match a
	# substring rather than the whole field.
	case "$ABI" in
	arm64-v8a) expected=AArch64 ;;
	x86_64)    expected=X86-64 ;;
	esac

	for so in $STAGING_AREA_DEPS/lib/*.so; do
		[ -e "$so" ] || continue
		local arch
		arch=$($READELF -h "$so" | awk -F: '/Machine:/ {gsub(/^ +| +$/,"",$2); print $2}')
		if ! echo "$arch" | grep -q -- "$expected"; then
			echo "FAIL: $(basename $so) is '$arch', expected '$expected'"
			fail=1
		fi
	done

	# Versioned or non-lib-prefixed files are unusable in an APK.
	local bad
	bad=$(find $STAGING_AREA_DEPS/lib -maxdepth 1 -name '*.so.*' -o -maxdepth 1 -name '*.so' -not -name 'lib*' 2>/dev/null)
	if [ -n "$bad" ]; then
		echo "FAIL: versioned or non-lib-prefixed libraries present:"
		echo "$bad"
		fail=1
	fi

	[ "$fail" == "0" ] && echo "### All libraries OK for $ABI"
	return $fail
}

# Emit the absolute .so paths for Scopy's QT_ANDROID_EXTRA_LIBS, which must list
# every third-party native library to be flattened into the APK's lib/<abi>/.
collect_extra_libs() {
	find $STAGING_AREA_DEPS/lib -maxdepth 1 -name 'lib*.so' | sort | paste -sd';' -
}

# Scopy itself, and the APK.
#
# This uses the Qt KIT toolchain file, not the NDK one set_cmake_opts() uses for the
# dependencies. Only qt.toolchain.cmake brings in Qt's Android deployment machinery - the
# scopy_make_apk / "apk" targets, androiddeployqt, Gradle - and it chainloads the NDK's own
# android.toolchain.cmake itself, from $ANDROID_NDK_ROOT. Its compiled-in fallback is
# /opt/android/android-ndk-r26b, which exists only in Qt's CI; if neither the env var nor -D is
# set it merely WARNS and then configures with no Android toolchain at all, so pass both
# ANDROID_*_ROOT explicitly.
#
# It deliberately does not reuse set_cmake_opts/build_with_cmake: those encode dependency-build
# policy that is wrong for Scopy - a staging install prefix, CMAKE_PLATFORM_NO_VERSIONED_SONAME,
# the NDK toolchain file, and make where the verified configuration used Ninja.
build_scopy() {
	print_config

	# android_toolchain.sh exports CFLAGS/CPPFLAGS/LDFLAGS for cross-building the DEPENDENCIES
	# (-I/-L into the staging prefix, -Wl,--undefined-version for NDK r26). CMake seeds
	# CMAKE_C_FLAGS and CMAKE_*_LINKER_FLAGS from the environment on the first configure, so
	# leaving them set would put the staging include dir ahead of Scopy's own headers.
	unset CFLAGS CPPFLAGS LDFLAGS

	# androiddeployqt only copies the libraries listed here, plus the app binary's recursive
	# closure - and CMakeLists.txt merely warns when the list is empty, which ships an APK that
	# starts with no plugins and no style. Fail here instead.
	local extra_libs
	extra_libs=$(collect_extra_libs)
	[ -n "$extra_libs" ] || {
		echo "ERROR: no lib*.so in $STAGING_AREA_DEPS/lib - run 'clone build_deps' first" >&2
		return 1
	}

	git config --global --add safe.directory $SRC_DIR

	# ENABLE_PLUGIN_ADC=OFF keeps gr-util out, and with it gnuradio/volk/genalyzer. SIGROK and
	# PYTHON have no Android dependency build; ENABLE_PLUGIN_DEBUGGER needs a local iio-emu
	# binary. M2K, TEST-PLUGINS and EXTPROC are already forced off in packages/CMakeLists.txt.
	# CMAKE_PREFIX_PATH intentionally omits $QT_ANDROID: qt.toolchain.cmake prepends the Qt
	# cmake dir itself, and listing it twice trips the path-collapsing bug that file documents.
	#
	# CMAKE_FIND_ROOT_PATH is NOT redundant with CMAKE_PREFIX_PATH. android.toolchain.cmake, which
	# qt.toolchain.cmake chainloads, sets CMAKE_FIND_ROOT_PATH_MODE_INCLUDE and _LIBRARY to ONLY, so
	# find_path/find_library search only beneath CMAKE_FIND_ROOT_PATH and re-root CMAKE_PREFIX_PATH
	# entries under it. Without the staging prefix here, CMakeLists.txt's find_path for iio.h fails
	# with IIO_INCLUDE_DIRS-NOTFOUND even though the header is installed. Only the staging prefix is
	# listed: android.toolchain.cmake appends the NDK sysroot to this list itself.
	local opts=(
		-G Ninja
		-DCMAKE_TOOLCHAIN_FILE="$QT_ANDROID/lib/cmake/Qt6/qt.toolchain.cmake"
		-DCMAKE_BUILD_TYPE="${BUILD_TYPE:-Release}"
		-DANDROID_ABI="$ABI"
		-DANDROID_PLATFORM="android-$API"
		-DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT"
		-DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT"
		-DQT_HOST_PATH="$QT_HOST_PATH"
		-DCMAKE_PREFIX_PATH="$STAGING_AREA_DEPS"
		-DCMAKE_FIND_ROOT_PATH="$STAGING_AREA_DEPS"
		-DSCOPY_ANDROID_EXTRA_LIBS="$extra_libs"
		-DENABLE_ALL_PACKAGES=ON
		-DENABLE_TESTING=OFF
		-DENABLE_SCOPYJS=ON
		-DENABLE_TRANSLATION=ON
		-DENABLE_PLUGIN_ADC=OFF
		-DENABLE_PLUGIN_DEBUGGER=OFF
		-DWITH_PYTHON=OFF
		-DWITH_SIGROK=OFF
		-DWITH_LIBSERIALPORT=ON
	)

	local build_dir=${SCOPY_BUILD_DIR:-$SRC_DIR/build-android-$ABI}
	mkdir -p $build_dir
	pushd $build_dir
	$CMAKE_BIN "${opts[@]}" $SRC_DIR
	# "apk" is the phony alias qt_finalize_executable() creates for scopy_make_apk, which depends
	# on scopy_android_package and through it on every plugin target - so this one target builds
	# Scopy, stages the assets and runs androiddeployqt + Gradle.
	$CMAKE_BIN --build . --target apk -- $JOBS
	popd
}

# Image-build entry point (ci/android/docker/Dockerfile), mirroring
# ci/ubuntu/ubuntu_build_process.sh configure_system. Everything it does is baked into the
# published image; CI then runs only build_scopy.
#
# build_deps ends in verify_deps, which returns non-zero on a wrong-architecture or versioned-
# SONAME library - so under this script's "set -e" a bad dependency fails the docker build
# rather than shipping a broken image.
configure_system() {
	download_sdk
	clone
	build_deps
}

for arg in $@; do
	$arg
done

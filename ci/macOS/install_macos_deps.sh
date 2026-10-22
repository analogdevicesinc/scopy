#!/bin/bash

# macOS Qt6 Build Script
# =====================================
# Build and install all the dependencies needed for Scopy Qt6 on macOS
# Usage: ./install_macos_deps.sh

set -ex
REPO_SRC=$(git rev-parse --show-toplevel)
source $REPO_SRC/ci/macOS/macos_config.sh

# GitHub Actions cache (see .github/workflows/macosbuild.yml): the workflow restores these and sets the *_CACHE_HIT vars
BREW_KEGS_DIR=$HOME/brew-kegs

# Copy every keg added or upgraded since BREW_BEFORE into BREW_KEGS_DIR, so the workflow can cache it
save_brew_kegs() {
	local prefix f keg
	prefix=$(brew --prefix)
	rm -rf $BREW_KEGS_DIR
	mkdir -p $BREW_KEGS_DIR
	touch $BREW_KEGS_DIR/formulae.txt
	for f in $(comm -13 <(echo "$BREW_BEFORE" | sort) <(brew list --formula --versions | sort) | cut -d ' ' -f 1); do
		keg=$(cd $prefix/opt/$f && pwd -P)
		mkdir -p $BREW_KEGS_DIR/$f
		cp -a $keg $BREW_KEGS_DIR/$f/
		# keg-only formulae are not linked, so restore links only what brew linked here
		if [ -L $prefix/var/homebrew/linked/$f ]; then
			echo "$f linked" >> $BREW_KEGS_DIR/formulae.txt
		else
			echo "$f" >> $BREW_KEGS_DIR/formulae.txt
		fi
	done
	cat $BREW_KEGS_DIR/formulae.txt
}

# Put the cached kegs back into the Cellar and recreate their opt/ and prefix links
restore_brew_kegs() {
	local prefix f linked keg
	prefix=$(brew --prefix)
	while read -r f linked <&3; do
		brew unlink $f 2>/dev/null || true
		rm -rf $prefix/Cellar/$f
		cp -a $BREW_KEGS_DIR/$f $prefix/Cellar/
		keg=$(ls $prefix/Cellar/$f)
		ln -sfn ../Cellar/$f/$keg $prefix/opt/$f
		if [ "$linked" == "linked" ]; then
			brew link --overwrite $f
		fi
	done 3< $BREW_KEGS_DIR/formulae.txt
}

OS_VERSION=${1:-$(sw_vers -productVersion)}
echo "MacOS version $OS_VERSION"
MACOS_MAJOR=$(echo "$OS_VERSION" | cut -d '.' -f 1)

# Qt6 via aqtinstall -- no Homebrew Qt package needed
# ghr is deliberately absent: it was never invoked (releases use actions/upload-artifact)
# and it has no x86_64 bottle, so on Intel it source-builds its `go` build dependency.
PACKAGES="pkg-config cmake fftw bison gettext autoconf automake libzip libusb doxygen wget gnu-sed dylibbundler libxml2 zstd"

install_packages() {

	# Workaround: Homebrew fails to upgrade Python's 2to3 due to conflicting symlinks  https://github.com/actions/runner-images/issues/6817
	rm -v /usr/local/bin/2to3* || true
	rm -v /usr/local/bin/idle3* || true
	rm -v /usr/local/bin/pydoc3* || true
	rm -v /usr/local/bin/python3* || true
	rm -v /usr/local/bin/python3-config || true
	rm -v /usr/local/bin/pip3.13 || true

	# uninstall cmake before installing other dependencies https://github.com/actions/runner-images/issues/12912
	brew uninstall --force cmake || true

	# macOS version gate: libtool is only appended to PACKAGES above macOS 12 (see below)
	macos_version=$(sw_vers -productVersion)
	major_version=$(echo "$macos_version" | cut -d '.' -f 1)

	if [ "$major_version" -gt 12 ]; then
		# Workaround: Install or update libtool package only if macOS version is greater than 12
		# Note: libtool (v2.4.7) is pre-installed by default, but it can be updated to v2.5.3
		PACKAGES="$PACKAGES libtool"
	fi

	if [ "${BREW_CACHE_HIT}" == "true" ]; then
		echo "Restoring cached Homebrew kegs (skipping update/install)..."
		export HOMEBREW_NO_AUTO_UPDATE=1
		restore_brew_kegs
	else
		echo "Installing packages fresh..."
		BREW_BEFORE=$(brew list --formula --versions)
		brew update

		if [ "$major_version" -gt 12 ]; then
			brew pin xcodes 2>/dev/null || true
			# No `brew upgrade` here on purpose: it upgraded every outdated package on the
			# runner image (41 on Intel, 29 on ARM), not just Scopy's. Homebrew stopped
			# building x86_64 bottles in Sept 2026, so 29 of those compiled from source on
			# Intel -- node@22 took 46 min and llvm@22 was still building when the 120 min
			# timeout killed the job. Scopy links none of them. See tasks/ci-macos-updates.md.
		fi

		brew install --overwrite --display-times $PACKAGES 2>&1 | grep -v "already installed and up-to-date"

		if [ "${GITHUB_ACTIONS}" == "true" ]; then
			save_brew_kegs
		fi
	fi

	pip3 install --break-system-packages mako
}

install_qt() {
	echo "### Installing Qt 6.8.3 via aqtinstall"
	pip3 install --break-system-packages aqtinstall
	# Use aqt directly (installed by pip3 in same bin dir) to avoid python3/pip3 interpreter mismatch
	aqt install-qt --outputdir $QT_INSTALL_LOCATION mac desktop 6.8.3 clang_64 -m qt3d qtscxml
}

patch_qt_macos26() {
	# Qt 6.8.3 predates Qt's AGL-removal fix (qtbase cdb33c3d, shipped 6.8.4+); AGL.framework is
	# gone from the macOS 26 SDK. Strip every AGL link reference ("-framework AGL" and AGL.framework
	# paths) across mkspecs, cmake modules/targets and framework .prl files so qmake and cmake link clean.
	[ "$MACOS_MAJOR" -ge 26 ] || return 0
	echo "### Patching Qt: removing AGL.framework references (dropped in macOS 26 SDK)"
	# FindWrapOpenGL.cmake also find_library()s AGL, which still resolves to the host's
	# /System/Library/Frameworks copy and becomes "-framework AGL" at link time. Drop the whole
	# AGL block, matching upstream qtbase 93ff6e6af4.
	sed -i '' \
		-e '/find_library(WrapOpenGL_AGL/,/^ *endif()/d' \
		-e '/if(NOT __opengl_agl_fw_path)/,/^ *endif()/d' \
		-e '/__opengl_agl_fw_path})/d' \
		"$QT/lib/cmake/Qt6/FindWrapOpenGL.cmake"
	local f
	for f in $(grep -rl "AGL" "$QT/mkspecs" "$QT/lib/cmake" 2>/dev/null; find "$QT/lib" -name "*.prl" 2>/dev/null); do
		sed -i '' \
			-e 's#-framework AGL##g' \
			-e 's#-weak_framework AGL##g' \
			-e 's#[^;" ]*/AGL\.framework/[^;" ]*##g' \
			"$f" 2>/dev/null || true
	done
	echo "### Verifying no AGL link references remain:"
	local leftover
	leftover=$( { grep -rnw "AGL" "$QT/mkspecs" "$QT/lib/cmake" 2>/dev/null; \
		find "$QT/lib" -name "*.prl" -exec grep -Hnw "AGL" {} + 2>/dev/null; } | grep -v ':[[:space:]]*#' || true)
	if [ -n "$leftover" ]; then
		echo "$leftover"
		echo "ERROR: AGL references remain in Qt after patching"
		exit 1
	fi
	echo "clean - no AGL references remain"
}

export_paths(){
	BREW_PREFIX="$(brew --prefix)"
	export PATH="$BREW_PREFIX/bin:$PATH"
	export PATH="$BREW_PREFIX/opt/bison/bin:$PATH"
	export PATH="${QT_PATH}:$PATH"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$BREW_PREFIX/opt/libzip/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$BREW_PREFIX/opt/libffi/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$STAGING_AREA_DEPS/lib/pkgconfig"

	QMAKE="$(command -v qmake6)"
	CMAKE_BIN="$(command -v cmake)"
	ARCH="$(uname -m)"
	CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGING_AREA_DEPS -DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_OSX_ARCHITECTURES=$ARCH"
	CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

	echo -- USING CMAKE COMMAND:
	echo $CMAKE
	echo -- USING QT: $QT_PATH
	echo -- USING QMAKE: $QMAKE
	echo -- PATH: $PATH
	echo -- PKG_CONFIG_PATH: $PKG_CONFIG_PATH
}

clone() {
	if [ "${DEPS_CACHE_HIT}" == "true" ]; then
		echo "Dependencies restored from cache - skipping clone"
		return 0
	fi

	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA

	git clone --recursive https://github.com/sigrokproject/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	git clone --recursive https://github.com/KDE/extra-cmake-modules.git -b $ECM_BRANCH extra-cmake-modules
	git clone --recursive https://github.com/KDE/karchive.git -b $KARCHIVE_BRANCH karchive
	git clone --recursive https://github.com/analogdevicesinc/genalyzer.git -b $GENALYZER_BRANCH genalyzer
	git clone --recursive https://github.com/qcoro/qcoro.git -b $QCORO_BRANCH qcoro
	if [ "$SCOPY_LIBIIO_VERSION" == "0" ]; then
		git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_V0_VERSION libiio
		git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
		git clone --recursive https://github.com/analogdevicesinc/libad9166-iio.git -b $LIBAD9166_BRANCH libad9166
		git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
		git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	else
		git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	fi

	popd
}

generate_status_file(){
	# Generate build status info for the about page
	BUILD_STATUS_FILE=${REPO_SRC}/build-status
	brew list --versions $PACKAGES > $BUILD_STATUS_FILE
	cat $STAGING_AREA_DEPS/deps-versions >> $BUILD_STATUS_FILE
}

# Kept inside STAGING_AREA_DEPS so it is cached together with the dependencies
save_version_info() {
	mkdir -p $STAGING_AREA_DEPS
	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
	>> $STAGING_AREA_DEPS/deps-versions
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
	make install
	popd
}

# libiio v1 as plain dylibs installed into STAGING_AREA_DEPS (OSX_FRAMEWORK/OSX_PACKAGE off), so dylibbundler
# bundles it like any other dependency. LIBIIO_COMPAT=OFF: nothing in the v1 build uses the libiio v0 API.
build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"
	CURRENT_BUILD=libiio
	pushd $STAGING_AREA/libiio
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="\
		-DWITH_TESTS:BOOL=OFF \
		-DWITH_MAN:BOOL=OFF \
		-DHAVE_DNS_SD:BOOL=ON \
		-DCSHARP_BINDINGS:BOOL=OFF \
		-DPYTHON_BINDINGS:BOOL=OFF \
		-DINSTALL_UDEV_RULE:BOOL=OFF \
		-DWITH_SERIAL_BACKEND:BOOL=ON \
		-DENABLE_IPV6:BOOL=OFF \
		-DOSX_FRAMEWORK:BOOL=OFF \
		-DOSX_PACKAGE:BOOL=OFF \
		-DWITH_IIOD_EMU:BOOL=ON \
		-DWITH_EMU_BACKEND:BOOL=ON \
		-DLIBIIO_COMPAT:BOOL=OFF \
		"
	build_with_cmake
	make install
	popd
}

build_libiio_v0() {
	echo "### Building libiio - version $LIBIIO_V0_VERSION"
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
		-DWITH_NETWORK_BACKEND=ON \
		-DENABLE_IPV6:BOOL=OFF \
		"
	build_with_cmake

	# manually install framework
	mkdir -p $STAGING_AREA_DEPS/include
	mkdir -p $STAGING_AREA_DEPS/lib/pkgconfig
	cp -v $STAGING_AREA/libiio/iio.h $STAGING_AREA_DEPS/include
	cp -avR $STAGING_AREA/libiio/build/iio.framework $STAGING_AREA_DEPS/lib
	cp -v $STAGING_AREA/libiio/build/libiio.pc $STAGING_AREA_DEPS/lib/pkgconfig
	popd
}


build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	CURRENT_BUILD=libad9361-iio
	pushd $STAGING_AREA/libad9361
	save_version_info
	build_with_cmake

	# manually install framework
	mkdir -p $STAGING_AREA_DEPS/include
	mkdir -p $STAGING_AREA_DEPS/lib/pkgconfig
	cp -v $STAGING_AREA/libad9361/ad9361.h $STAGING_AREA_DEPS/include
	cp -avR $STAGING_AREA/libad9361/build/ad9361.framework $STAGING_AREA_DEPS/lib
	cp -v $STAGING_AREA/libad9361/build/libad9361.pc $STAGING_AREA_DEPS/lib/pkgconfig
	popd
}

build_libad9166() {
	echo "### Building libad9166 - branch $LIBAD9166_BRANCH"
	CURRENT_BUILD=libad9166-iio
	pushd $STAGING_AREA/libad9166
	save_version_info
	build_with_cmake

	# manually install framework (OSX_FRAMEWORK installs to /Library/Frameworks, not CMAKE_INSTALL_PREFIX)
	mkdir -p $STAGING_AREA_DEPS/include
	mkdir -p $STAGING_AREA_DEPS/lib/pkgconfig
	cp -v $STAGING_AREA/libad9166/ad9166.h $STAGING_AREA_DEPS/include
	cp -avR $STAGING_AREA/libad9166/build/ad9166.framework $STAGING_AREA_DEPS/lib
	cp -v $STAGING_AREA/libad9166/build/libad9166.pc $STAGING_AREA_DEPS/lib/pkgconfig
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
+    macx: QWT_SONAME=\$\${QWT_INSTALL_LIBS}/libqwt_scopy.dylib
 }
 else {
     CONFIG += staticlib
EOF
}


build_libm2k() {
	echo "### Building libm2k - branch $LIBM2K_BRANCH"
	pushd $STAGING_AREA/libm2k
	CURRENT_BUILD=libm2k
	save_version_info

	# libm2k hardcodes x86_64 in src/CMakeLists.txt -- patch to match runner architecture
	ARCH="$(uname -m)"
	sed -i '' "s/set(CMAKE_OSX_ARCHITECTURES \"x86_64\")/set(CMAKE_OSX_ARCHITECTURES \"$ARCH\")/" src/CMakeLists.txt

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

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	CURRENT_BUILD=qwt
	pushd $STAGING_AREA/qwt
	save_version_info
	git clean -xdf
	git reset --hard
	patch_qwt
	# Rename the produced library base name to qwt_scopy (SONAME handled in patch_qwt)
	sed -i '' 's|qwtLibraryTarget(qwt)|qwtLibraryTarget(qwt_scopy)|' src/src.pro
	# Fix subproject link references so they find qwt_scopy, not the old qwt name
	sed -i '' 's|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt)|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt_scopy)|' \
		designer/designer.pro examples/examples.pri playground/playground.pri tests/tests.pri
	$QMAKE_BIN INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib \
		QMAKE_CXXFLAGS+="-Wno-implicit-function-declaration" qwt.pro
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
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$QT -DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DKDDockWidgets_QT6=ON -DKDDockWidgets_FRONTENDS=qtwidgets"
	build_with_cmake
	make install
	popd
}

build_ecm() {
	echo "### Building extra-cmake-modules (ECM) - branch $ECM_BRANCH"
	pushd $STAGING_AREA/extra-cmake-modules
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DBUILD_QTHELP_DOCS=OFF"
	build_with_cmake
	make install
	popd
}

build_karchive () {
	echo "### Building karchive - version $KARCHIVE_BRANCH"
	pushd $STAGING_AREA/karchive
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$QT -DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DBUILD_TESTING=OFF"
	build_with_cmake
	make install
	popd
}

build_genalyzer() {
	echo "### Building genalyzer - branch $GENALYZER_BRANCH"
	CURRENT_BUILD=genalyzer
	pushd $STAGING_AREA/genalyzer
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="\
		-DBUILD_TESTING=OFF \
		-DBUILD_SHARED_LIBS=ON \
		"
	build_with_cmake
	make install
	popd
}

build_qcoro() {
	echo "### Building qcoro - version $QCORO_BRANCH"
	CURRENT_BUILD=qcoro
	pushd $STAGING_AREA/qcoro
	# QtDBus auto-disabled on APPLE; Network via qtbase. Qt found via $QT (aqt install), like build_kddock.
	CURRENT_BUILD_CMAKE_OPTS="\
		-DCMAKE_PREFIX_PATH=$QT \
		-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS \
		-DQCORO_BUILD_EXAMPLES=OFF \
		-DQCORO_BUILD_TESTING=OFF \
		-DBUILD_TESTING=OFF \
		-DQCORO_WITH_QTWEBSOCKETS=OFF \
		-DQCORO_WITH_QTQUICK=OFF \
		-DQCORO_WITH_QML=OFF \
		-DBUILD_SHARED_LIBS=ON \
		"
	build_with_cmake
	make install
	popd
}

build_deps(){
	if [ "${DEPS_CACHE_HIT}" == "true" ]; then
		echo "Dependencies restored from cache in $STAGING_AREA_DEPS"
		return 0
	fi

	echo "Building all dependencies from source..."
	rm -f $STAGING_AREA_DEPS/deps-versions

	build_libserialport
	if [ "$SCOPY_LIBIIO_VERSION" == "0" ]; then
		# libiio v0 and its consumers
		build_libiio_v0
		build_libad9361
		build_libad9166
		build_libm2k
		build_libtinyiiod
	else
		build_libiio
	fi
	build_qwt
	build_kddock
	build_ecm
	build_karchive
	build_genalyzer
	build_qcoro
}

# Install Qt6 via aqtinstall, install dependencies, clone repositories, and build all dependencies
install_packages
install_qt
export_paths
patch_qt_macos26
clone
build_deps
generate_status_file

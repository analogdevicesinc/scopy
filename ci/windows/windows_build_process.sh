#!/usr/bin/bash.exe

set -xe
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

USE_STAGING=OFF

source $SCRIPT_DIR/mingw_toolchain.sh $USE_STAGING

install_packages() {
	SYSTEM_PKGS="\
		git\
		svn\
		vim\
		unzip\
		zip\
		pkg-config\
	"

	TOOLS_PKGS="\
		mingw-w64-${ARCH}-wget2\
		mingw-w64-${ARCH}-cmake\
		mingw-w64-${ARCH}-gcc\
		mingw-w64-${ARCH}-python3\
		mingw-w64-${ARCH}-python-mako\
		mingw-w64-${ARCH}-python-pip\
		mingw-w64-${ARCH}-make\
		mingw-w64-${ARCH}-doxygen\
		mingw-w64-${ARCH}-pcre2\
		base-devel\
		mingw-w64-${ARCH}-autotools\
		libtool\
		mingw-w64-${ARCH}-ccache \
		mingw-w64-${ARCH}-pkgconf
	"

	PACMAN_SYNC_DEPS="\
		mingw-w64-${ARCH}-fftw\
		mingw-w64-${ARCH}-libxml2\
		mingw-w64-${ARCH}-libzip\
		mingw-w64-${ARCH}-libffi\
		mingw-w64-${ARCH}-doxygen\
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

	download_cmake
}

# install_qt() {

# 	pacman --noconfirm -S mingw-w64-x86_64-zstd wget

# 	echo "Downloading standalone aqt binary..."
# 	wget -qO aqt.exe https://github.com/miurahr/aqtinstall/releases/latest/download/aqt_x64.exe
# 	chmod +x aqt.exe

# 	echo "Installing Qt6..."
# 	./aqt.exe install-qt --outputdir /c/Qt windows desktop 6.8.3 win64_mingw -m qt3d qtscxml

# 	# # Install the pre-compiled MSYS2 versions of the failing dependencies
# 	# pacman --noconfirm -S mingw-w64-x86_64-zstd \
# 	# 	mingw-w64-x86_64-python-psutil \
# 	# 	mingw-w64-x86_64-python-zstandard

# 	# # Force pip to bypass the PEP 668 environment lock
# 	# pip3 install aqtinstall --break-system-packages

# 	# python3 -m aqt install-qt --outputdir /c/Qt windows desktop 6.8.3 win64_mingw -m qt3d qtscxml
# }

install_qt() {
	pacman --noconfirm -S mingw-w64-x86_64-zstd wget

	# aqt.exe is a native Windows (PyInstaller) binary, so it resolves its cache directory with
	# Python's Windows expanduser - which needs USERPROFILE, or HOMEDRIVE+HOMEPATH. A plain
	# `bash -lc` MSYS2 login shell exports none of them and aqt dies with "Could not determine
	# home directory". Derive USERPROFILE from $HOME only when it is missing, so environments
	# that already set it (e.g. the docker image) are untouched.
	if [ -z "$USERPROFILE" ]; then
		export USERPROFILE=$(cygpath -w "$HOME")
		echo "USERPROFILE was unset; using $USERPROFILE for aqt"
	fi

	# Download into the staging area rather than the current directory, which would otherwise
	# leave a ~15MB aqt.exe sitting in the source tree.
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA

	echo "Downloading standalone aqt binary..."
	wget -qO aqt.exe https://github.com/miurahr/aqtinstall/releases/latest/download/aqt_x64.exe
	chmod +x aqt.exe

	echo "Installing Qt6..."
	# Changed /c/Qt to C:/Qt below
	# qt3d kept for the imuanalyzer plugin; qtscxml dropped (no Scopy code uses Scxml).
	./aqt.exe install-qt --outputdir C:/Qt windows desktop 6.8.3 win64_mingw -m qt3d

	popd
}

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA
	[ -d 'libserialport' ] || git clone --recursive https://github.com/sigrokproject/libserialport -b $LIBSERIALPORT_BRANCH libserialport
	[ -d 'libiio' ]		|| git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
	[ -d 'libad9361' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
	[ -d 'libad9166' ]	|| git clone --recursive https://github.com/analogdevicesinc/libad9166-iio.git -b $LIBAD9166_BRANCH libad9166
	[ -d 'libsndfile' ]	|| git clone --recursive https://github.com/libsndfile/libsndfile -b $LIBSNDFILE_BRANCH libsndfile
	[ -d 'qwt' ]		|| git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
	[ -d 'libtinyiiod' ]	|| git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
	[ -d 'KDDockWidgets' ] || git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
	[ -d 'extra-cmake-modules' ] || git clone --recursive https://github.com/KDE/extra-cmake-modules.git -b $ECM_BRANCH extra-cmake-modules
	[ -d 'karchive' ] || git clone --recursive https://github.com/KDE/karchive.git -b $KARCHIVE_BRANCH karchive
	[ -d 'genalyzer' ] || git clone --recursive https://github.com/analogdevicesinc/genalyzer.git -b $GENALYZER_BRANCH genalyzer
	popd
}

create_build_status_file() {
	touch $BUILD_STATUS_FILE
	echo "Scopy2-MinGW-Qt6" >> $BUILD_STATUS_FILE
	echo "Docker image built on $(date)" >> $BUILD_STATUS_FILE
	echo "Deps installed using pacman" >> $BUILD_STATUS_FILE
	echo "Qt6 installed via aqtinstall at $QT" >> $BUILD_STATUS_FILE
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

	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE

	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

	popd

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

	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE
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

build_libad9166() {
	echo "### Building libad9166 - branch $LIBAD9166_BRANCH"
	CURRENT_BUILD=libad9166
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

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	CURRENT_BUILD=qwt
	pushd $STAGING_AREA/$CURRENT_BUILD
	git clean -xdf

	local PREV_JOBS=$JOBS
	JOBS="-j2"


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

	# Rename the produced library base name to qwt_scopy (no SONAME on Windows)
	sed -i 's|qwtLibraryTarget(qwt)|qwtLibraryTarget(qwt_scopy)|' src/src.pro
	# The designer/examples/playground/tests subprojects link the library by its old name
	# (qwtAddLibrary(..., qwt)); update those to match the rename above, otherwise they fail
	# with `ld: cannot find -lqwt`. (Gap in PR #2291; already fixed for Ubuntu.)
	sed -i 's|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt)|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt_scopy)|' \
		designer/designer.pro examples/examples.pri playground/playground.pri tests/tests.pri

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

	cp $STAGING_DIR/lib/qwt_scopy.dll $STAGING_DIR/bin/qwt_scopy.dll

	if [ "$INSTALL" == "ON" ] && [ "$CI_SCRIPT" == "ON" ];then
		git clean -xdf
	fi

		# Restore original jobs variable for the rest of the script
	JOBS=$PREV_JOBS

	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	CURRENT_BUILD=libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"

	local PREV_JOBS=$JOBS
	JOBS="-j2"

	build_with_cmake $1

	# Restore original jobs variable for the rest of the script
	JOBS=$PREV_JOBS
}

build_kddock () {
	echo "### Building KDDockWidgets - version $KDDOCK_BRANCH"
	CURRENT_BUILD=KDDockWidgets
	# KDDockWidgets_NO_SPDLOG=ON is REQUIRED, not cosmetic. KDDockWidgets does
	# find_package(spdlog QUIET) and links spdlog::spdlog if it finds it. On Windows spdlog is
	# always present because it is a dependency of the mingw doxygen package, so without this the
	# library links libspdlog and the bundle needs libspdlog-*.dll at runtime - Scopy.exe then
	# fails to start with STATUS_DLL_NOT_FOUND (0xC0000135). Ubuntu only avoids this by accident,
	# spdlog simply not being installed there. Being explicit makes it deterministic.
	CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON -DKDDockWidgets_FRONTENDS=qtwidgets -DKDDockWidgets_EXAMPLES=OFF -DKDDockWidgets_TESTS=OFF -DKDDockWidgets_NO_SPDLOG=ON"

		local PREV_JOBS=$JOBS
	JOBS="-j2"

	build_with_cmake $1

	# Restore original jobs variable for the rest of the script
	JOBS=$PREV_JOBS
}

build_ecm() {
	echo "### Building extra-cmake-modules (ECM) - branch $ECM_BRANCH"
	CURRENT_BUILD=extra-cmake-modules
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DBUILD_QTHELP_DOCS=OFF"

		local PREV_JOBS=$JOBS
	JOBS="-j2"

	build_with_cmake $1

	# Restore original jobs variable for the rest of the script
	JOBS=$PREV_JOBS
}

build_karchive () {
	echo "### Building karchive - version $KARCHIVE_BRANCH"
	CURRENT_BUILD=karchive
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_TESTING=OFF"

		local PREV_JOBS=$JOBS
	JOBS="-j2"

	build_with_cmake $1

	# Restore original jobs variable for the rest of the script
	JOBS=$PREV_JOBS
}

build_genalyzer() {
	echo "### Building genalyzer - branch $GENALYZER_BRANCH"
	CURRENT_BUILD=genalyzer
	CURRENT_BUILD_CMAKE_OPTS="\
		-DBUILD_TESTING=OFF \
		-DBUILD_SHARED_LIBS=ON \
		"

		local PREV_JOBS=$JOBS
	JOBS="-j2"

	build_with_cmake $1

	# Restore original jobs variable for the rest of the script
	JOBS=$PREV_JOBS
}

build_deps() {
	install_packages
	install_qt
	create_build_status_file
	clone

	build_libserialport ON
	build_libiio ON
	build_libad9361 ON
	build_libad9166 ON
	build_libsndfile ON
	build_qwt ON
	build_libtinyiiod ON
	build_kddock ON
	build_ecm ON
	build_karchive ON
	build_genalyzer ON
}

# Run named steps if any were given, otherwise do the full dependency build. The docker image
# invokes this with no arguments, so its behaviour is unchanged; passing a step name now runs
# only that step, which is what a local/incremental build needs.
if [ $# -gt 0 ]; then
	for arg in $@; do
		$arg
	done
else
	build_deps
fi

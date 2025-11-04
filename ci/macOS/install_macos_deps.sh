#!/bin/bash

set -ex
REPO_SRC=$(git rev-parse --show-toplevel)
source $REPO_SRC/ci/macOS/macos_config.sh

# Cache configuration
CACHE_BASE_DIR="${PIPELINE_WORKSPACE}"
GIT_CACHE_DIR="${CACHE_BASE_DIR}/.git-cache"
HOMEBREW_CACHE_DIR="${CACHE_BASE_DIR}/.homebrew-cache"
CACHE_SIZE_WARNING_GB=8

PACKAGES="${QT_FORMULAE} volk spdlog ${BOOST_FORMULAE} pkg-config cmake fftw bison gettext autoconf automake libzip glib libusb glog doxygen wget gnu-sed libmatio dylibbundler libxml2 ghr libsndfile"

# Cache size monitoring
check_cache_sizes() {
    if [ -d "$CACHE_BASE_DIR" ]; then
        local total_size_gb=$(du -sg "$CACHE_BASE_DIR" 2>/dev/null | cut -f1 || echo "0")
        echo "Total cache size: ${total_size_gb}GB"

        if [ "$total_size_gb" -gt "$CACHE_SIZE_WARNING_GB" ]; then
            echo "⚠️  WARNING: Cache size (${total_size_gb}GB) approaching Azure DevOps limits"
        fi
    fi
}


# Cache cleanup function
cleanup_old_caches() {
    echo "Cleaning up old cache directories..."

    # Remove any temporary cache directories older than 7 days
    if [ -d "$CACHE_BASE_DIR" ]; then
        find "$CACHE_BASE_DIR" -name "*.tmp" -type d -mtime +7 -exec rm -rf {} + 2>/dev/null || true
        find "$CACHE_BASE_DIR" -name "*.old" -type d -mtime +7 -exec rm -rf {} + 2>/dev/null || true
    fi
}

setup_homebrew_cache() {
	if [ "${CACHING_ENABLED}" = "true" ]; then
		echo "Setting up Homebrew cache..."
		mkdir -p "$HOMEBREW_CACHE_DIR"

		export HOMEBREW_CACHE="$HOMEBREW_CACHE_DIR"
		#check if cache for homebrew exists
		if [ "${HOMEBREW_CACHE}" = "true" ]; then
			echo "Homebrew cache found, restoring cached packages"
			export HOMEBREW_NO_AUTO_UPDATE=1
		else
			echo "No Homebrew cache - downloading fresh packages"
		fi
	fi
}

setup_git_cache() {
	if [ "${CACHING_ENABLED}" = "true" ]; then
		echo "Setting up Git cache..."
		mkdir -p "$GIT_CACHE_DIR"

		if [ "${GIT_REPOS_CACHE}" = "true" ]; then
			echo "Git repositories cache found, restoring cached repositories"
			# Validate cache isn't corrupted
			if [ -d "$GIT_CACHE_DIR" ] && [ "$(ls -A $GIT_CACHE_DIR 2>/dev/null)" ]; then
				export GIT_CACHE_ENABLED=true
			else
				echo "Git cache directory empty - clearing and rebuilding"
				rm -rf "$GIT_CACHE_DIR"
				mkdir -p "$GIT_CACHE_DIR"
				export GIT_CACHE_ENABLED=false
			fi
		else
			echo "No Git cache - cloning fresh repositories"
			export GIT_CACHE_ENABLED=false
		fi
	fi
}

setup_dependencies_cache() {
    if [ "${CACHING_ENABLED}" = "true" ] && [ "${BUILT_DEPS_CACHE}" = "true" ]; then
        echo "Built dependencies cache found, restoring cached dependencies"
        # Validate cache isn't corrupted
        if [ -d "$STAGING_AREA_DEPS" ] && [ "$(ls -A $STAGING_AREA_DEPS 2>/dev/null)" ]; then
            echo "Found cached dependencies in $STAGING_AREA_DEPS"
            export DEPENDENCIES_CACHED=true
        else
            echo "Dependencies cache directory empty or corrupted - clearing and rebuilding"
            rm -rf "$STAGING_AREA_DEPS"
            export DEPENDENCIES_CACHED=false
        fi
    else
        echo "No dependencies cache - building fresh"
        export DEPENDENCIES_CACHED=false
    fi
    
}

OS_VERSION=${1:-$(sw_vers -productVersion)}
echo "MacOS version $OS_VERSION"

source ${REPO_SRC}/ci/macOS/before_install_lib.sh

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

	# Workaround for brew taking a long time to upgrade existing packages
	# Check if macOS version and upgrade packages only if the version is greater than macOS 12
	macos_version=$(sw_vers -productVersion)
	major_version=$(echo "$macos_version" | cut -d '.' -f 1)
	
	# Package installation based on cache status
	if [ "${CACHING_ENABLED}" = "true" ] && [ "${HOMEBREW_CACHE}" = "true" ]; then
		echo "Installing packages with cache optimization (skipping update/upgrade)..."
		export HOMEBREW_NO_AUTO_UPDATE=1
	else
		echo "Installing packages fresh..."
		brew update

		if [ "$major_version" -gt 12 ]; then
			brew upgrade --display-times || true #ignore homebrew upgrade errors
			# Workaround: Install or update libtool package only if macOS version is greater than 12
			# Note: libtool (v2.4.7) is pre-installed by default, but it can be updated to v2.5.3
			PACKAGES="$PACKAGES libtool"
		fi
	fi


	brew install --overwrite --display-times $PACKAGES

	pip3 install --break-system-packages mako
}

export_paths(){
	QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
	export PATH="/usr/local/bin:$PATH"
	export PATH="/usr/local/opt/bison/bin:$PATH"
	export PATH="${QT_PATH}:$PATH"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libzip/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$STAGING_AREA_DEPS/lib/pkgconfig"

	QMAKE="$(command -v qmake)"
	CMAKE_BIN="$(command -v cmake)"
	CMAKE_OPTS="-DCMAKE_PREFIX_PATH=$STAGING_AREA_DEPS -DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DCMAKE_POLICY_VERSION_MINIMUM=3.5"
	CMAKE="$CMAKE_BIN ${CMAKE_OPTS[*]}"

	echo -- USING CMAKE COMMAND:
	echo $CMAKE
	echo -- USING QT: $QT_PATH
	echo -- USING QMAKE: $QMAKE
	echo -- PATH: $PATH
	echo -- PKG_CONFIG_PATH: $PKG_CONFIG_PATH
}

clone() {
	echo "#######CLONE#######"
	mkdir -p $STAGING_AREA
	pushd $STAGING_AREA

	if [ "${CACHING_ENABLED}" = "true" ] && [ "$GIT_CACHE_ENABLED" = "true" ]; then
		echo "Using cached repositories..."

		# If cache directory has content, use it
		if [ -d "$GIT_CACHE_DIR" ] && [ "$(ls -A $GIT_CACHE_DIR 2>/dev/null)" ]; then
			echo "Copying cached repositories"
			cp -r "$GIT_CACHE_DIR"/* .
		else
			echo "Git cache directory empty - cloning fresh"
			export GIT_CACHE_ENABLED=false
		fi
	fi

	if [ "$GIT_CACHE_ENABLED" = "false" ]; then
		echo "Cloning all repositories fresh..."

		git clone --recursive https://github.com/sigrokproject/libserialport -b $LIBSERIALPORT_BRANCH libserialport
		git clone --recursive https://github.com/analogdevicesinc/libiio.git -b $LIBIIO_VERSION libiio
		git clone --recursive https://github.com/analogdevicesinc/libad9361-iio.git -b $LIBAD9361_BRANCH libad9361
		git clone --recursive https://github.com/analogdevicesinc/libm2k.git -b $LIBM2K_BRANCH libm2k
		git clone --recursive https://github.com/analogdevicesinc/gr-scopy.git -b $GRSCOPY_BRANCH gr-scopy
		git clone --recursive https://github.com/analogdevicesinc/gr-m2k.git -b $GRM2K_BRANCH gr-m2k
		git clone --recursive https://github.com/analogdevicesinc/gnuradio.git -b $GNURADIO_BRANCH gnuradio
		git clone --recursive https://github.com/cseci/qwt.git -b $QWT_BRANCH qwt
		git clone --recursive https://github.com/sigrokproject/libsigrokdecode.git -b $LIBSIGROKDECODE_BRANCH libsigrokdecode
		git clone --recursive https://github.com/analogdevicesinc/libtinyiiod.git -b $LIBTINYIIOD_BRANCH libtinyiiod
		git clone --recursive https://github.com/KDAB/KDDockWidgets.git -b $KDDOCK_BRANCH KDDockWidgets
		git clone --recursive https://github.com/KDE/extra-cmake-modules.git -b $ECM_BRANCH extra-cmake-modules
		git clone --recursive https://github.com/KDE/karchive.git -b $KARCHIVE_BRANCH karchive

		DEPENDENCY_REPOS="libserialport libiio libad9361 libm2k gr-scopy gr-m2k gnuradio qwt libsigrokdecode libtinyiiod KDDockWidgets extra-cmake-modules karchive"
		# Save to cache for next time
		if [ -n "$GIT_CACHE_DIR" ]; then
			mkdir -p "$GIT_CACHE_DIR"
			for repo in $DEPENDENCY_REPOS; do
				cp -r "$repo" "$GIT_CACHE_DIR/"
			done
		fi
	fi

	popd
}

generate_status_file(){
	# Generate build status info for the about page
	BUILD_STATUS_FILE=${REPO_SRC}/build-status
	brew list --versions $PACKAGES > $BUILD_STATUS_FILE
}

save_version_info() {
	echo "$(basename -a "$(git config --get remote.origin.url)") - $(git rev-parse --abbrev-ref HEAD) - $(git rev-parse --short HEAD)" \
	>> $BUILD_STATUS_FILE
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

build_libiio() {
	echo "### Building libiio - version $LIBIIO_VERSION"
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

build_libm2k() {
	echo "### Building libm2k - branch $LIBM2K_BRANCH"
	pushd $STAGING_AREA/libm2k
	CURRENT_BUILD=libm2k
	save_version_info

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

build_gnuradio() {
	echo "### Building gnuradio - branch $GNURADIO_BRANCH"
	CURRENT_BUILD=gnuradio
	pushd $STAGING_AREA/gnuradio
	save_version_info
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
	build_with_cmake
	make install
	popd
}

build_grm2k() {
	echo "### Building gr-m2k - branch $GRM2K_BRANCH"
	CURRENT_BUILD=gr-m2k
	pushd $STAGING_AREA/gr-m2k
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF \
		-DDIGITAL=OFF
		"
	build_with_cmake
	make install
	popd
}

build_grscopy() {
	echo "### Building gr-scopy - branch $GRSCOPY_BRANCH"
	CURRENT_BUILD=gr-scopy
	pushd $STAGING_AREA/gr-scopy
	save_version_info
	CURRENT_BUILD_CMAKE_OPTS="-DWITH_PYTHON=OFF "
	build_with_cmake
	make install
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	CURRENT_BUILD=libsigrokdecode
	pushd $STAGING_AREA/libsigrokdecode
	save_version_info
	git reset --hard
	git clean -xdf
	./autogen.sh
	./configure --prefix $STAGING_AREA_DEPS
	make $JOBS install
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
+    macx: QWT_SONAME=\$\${QWT_INSTALL_LIBS}/libqwt.dylib
 }
 else {
     CONFIG += staticlib
EOF
}


build_qwt() {
	echo "### Building qwt - branch qwt-multiaxes"
	CURRENT_BUILD=qwt
	pushd $STAGING_AREA/qwt
	save_version_info
	git clean -xdf
	git reset --hard
	patch_qwt
	$QMAKE INCLUDEPATH=$STAGING_AREA_DEPS/include LIBS=-L$STAGING_AREA_DEPS/lib qwt.pro
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
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS"
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
	CURRENT_BUILD_CMAKE_OPTS="-DCMAKE_INSTALL_PREFIX=$STAGING_AREA_DEPS -DBUILD_TESTING=OFF"
	build_with_cmake
	make install
	popd
}

build_deps(){
	if [ "$DEPENDENCIES_CACHED" = "true" ]; then
		echo "Found cached dependencies in $STAGING_AREA_DEPS"
		return 0
	fi

	echo "Building all dependencies from source..."

	build_libserialport
	build_libiio
	build_libad9361
	build_libm2k
	build_gnuradio
	build_grscopy
	build_grm2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
	build_kddock
	build_ecm
	build_karchive	
}

# Setup cache management
cleanup_old_caches
setup_homebrew_cache
setup_git_cache
setup_dependencies_cache
check_cache_sizes

install_packages
export_paths
clone
generate_status_file
build_deps

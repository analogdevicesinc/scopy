#!/usr/bin/bash.exe
set -xe
# get the full directory path of the script
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SRC_DIR/mingw_toolchain.sh $1 $2

TOOLS_PKGS="\
	mingw-w64-${ARCH}-cmake\
	mingw-w64-${ARCH}-gcc\
	mingw-w64-${ARCH}-python3\
	mingw-w64-${ARCH}-python-mako\
	mingw-w64-${ARCH}-python-six\
	mingw-w64-${ARCH}-make\
	mingw-w64-${ARCH}-doxygen\
	mingw-w64-${ARCH}-pcre2\
	git\
	svn\
	vim\
	base-devel\
	mingw-w64-${ARCH}-autotools\
	libtool\
"
	#mingw-w64-${ARCH}-boost 
PACMAN_SYNC_DEPS="\
	mingw-w64-${ARCH}-fftw\
	mingw-w64-${ARCH}-orc\
	mingw-w64-${ARCH}-libxml2\
	mingw-w64-${ARCH}-libzip\
	mingw-w64-${ARCH}-fftw\
	mingw-w64-${ARCH}-libzip\
	mingw-w64-${ARCH}-libffi\
	mingw-w64-${ARCH}-glib2\
	mingw-w64-${ARCH}-glibmm\
	mingw-w64-${ARCH}-doxygen\
	mingw-w64-${ARCH}-qt5\
	mingw-w64-${ARCH}-zlib\
	mingw-w64-${ARCH}-breakpad\
	mingw-w64-${ARCH}-libusb\
	mingw-w64-${ARCH}-libserialport\
"

install_tools() {
	$PACMAN -S unzip zip
	pushd $WORKFOLDER
	if [ ! -f windres.exe ]; then
		wget http://swdownloads.analog.com/cse/build/windres.exe.gz
		gunzip windres.exe.gz
	fi

	if [ ! -f dpinst.zip ]; then
		wget http://swdownloads.analog.com/cse/m1k/drivers/dpinst.zip
		unzip "dpinst.zip"
	fi

	if [ ! -f dfu-util.zip ]; then
		wget http://swdownloads.analog.com/cse/m1k/drivers/dfu-util.zip
		unzip "dfu-util.zip"
	fi

	if [ ! -f cv2pdb-dlls.zip ]; then
		wget https://swdownloads.analog.com/cse/scopydeps/cv2pdb-dlls.zip
		unzip "cv2pdb-dlls.zip"
	fi

	if [ ! -f is.exe ]; then
		wget https://jrsoftware.org/download.php/is.exe
	fi
	$PACMAN -S $TOOLS_PKGS
	popd
}

install_deps() {
	$PACMAN -S $PACMAN_SYNC_DEPS
	$PACMAN -U https://repo.msys2.org/mingw/${ARCH}/mingw-w64-${ARCH}-boost-1.79.0-4-any.pkg.tar.zst
}

recurse_submodules() {
	pushd $WORKFOLDER
	git submodule update --init --recursive --jobs 9
	popd
}

create_build_status_file() {
	echo "Built scopy-mingw-build-deps on Appveyor" >> $BUILD_STATUS_FILE
	echo "on $(date)" >> $BUILD_STATUS_FILE
	echo "url: ${APPVEYOR_URL}" >> $BUILD_STATUS_FILE
	echo "api_url: ${APPVEYOR_API_URL}" >> $BUILD_STATUS_FILE
	echo "acc_name: ${APPVEYOR_ACCOUNT_NAME}" >> $BUILD_STATUS_FILE
	echo "prj_name: ${APPVEYOR_PROJECT_NAME}" >> $BUILD_STATUS_FILE
	echo "build_id: ${APPVEYOR_BUILD_ID}" >> $BUILD_STATUS_FILE
	echo "build_nr: ${APPVEYOR_BUILD_NUMBER}" >> $BUILD_STATUS_FILE
	echo "build_version: ${APPVEYOR_BUILD_VERSION}" >> $BUILD_STATUS_FILE
	echo "job_id: ${APPVEYOR_JOB_ID}" >> $BUILD_STATUS_FILE
	echo "job_name: ${APPVEYOR_JOB_NAME}" >> $BUILD_STATUS_FILE
	echo "job_nr: ${APPVEYOR_JOB_NUMBER}" >> $BUILD_STATUS_FILE
	echo "job_link:  ${APPVEYOR_URL}/project/${APPVEYOR_ACCOUNT_NAME}/${APPVEYOR_PROJECT_NAME}/builds/${APPVEYOR_BUILD_ID}/job/${APPVEYOR_JOB_ID}" >> $BUILD_STATUS_FILE

	echo $BUILD_STATUS_FILE

	echo "Repo deps locations/files" >> $BUILD_STATUS_FILE
	echo $PACMAN_REPO_DEPS >> $BUILD_STATUS_FILE
	#ls ${WORKFOLDER}/old_msys_deps_${MINGW_VERSION}
}

__clean_build_dir() {
	git clean -xdf
	rm -rf ${WORKFOLDER}/${CURRENT_BUILD}/build-${ARCH}
	mkdir ${WORKFOLDER}/${CURRENT_BUILD}/build-${ARCH}
	cd ${WORKFOLDER}/${CURRENT_BUILD}/build-${ARCH}
}

__build_with_cmake() {
	INSTALL="install"
	[ -z $NO_INSTALL ] || INSTALL=""
	pushd $WORKFOLDER/$CURRENT_BUILD
	__clean_build_dir
	eval $CURRENT_BUILD_POST_CLEAN
	eval $CURRENT_BUILD_PATCHES
	$CMAKE $CURRENT_BUILD_CMAKE_OPTS $WORKFOLDER/$CURRENT_BUILD
	eval $CURRENT_BUILD_POST_CMAKE
	make $JOBS $INSTALL
	eval $CURRENT_BUILD_POST_MAKE		
	echo "$CURRENT_BUILD - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd

	#clean deps folder
	[ -z $INSTALL ] || rm -rf ${WORKFOLDER}/${CURRENT_BUILD}/build-${ARCH}

	# clear vars
	CURRENT_BUILD_CMAKE_OPTS="" 
	CURRENT_BUILD_POST_CLEAN=""
	CURRENT_BUILD_PATCHES=""
	CURRENT_BUILD_POST_CMAKE=""
	CURRENT_BUILD_POST_MAKE=""
	CURRENT_BUILD=""
	NO_INSTALL=""
}

build_libiio() {
	CURRENT_BUILD=libiio
	CURRENT_BUILD_CMAKE_OPTS="\
		${RC_COMPILER_OPT}\
		-DWITH_USB_BACKEND:BOOL=ON\
		-DWITH_SERIAL_BACKEND:BOOL=ON\
		-DWITH_TESTS:BOOL=ON\
		-DWITH_DOC:BOOL=OFF\
		-DCSHARP_BINDINGS:BOOL=OFF\
		-DPYTHON_BINDINGS:BOOL=OFF\
		-DHAVE_DNS_SD:BOOL=ON\
		-DENABLE_IPV6:BOOL=OFF\
	"
	__build_with_cmake
}

build_libad9361() {
	echo "### Building libad9361 - branch $LIBAD9361_BRANCH"
	CURRENT_BUILD=libad9361
	__build_with_cmake
}

build_libm2k() {
	CURRENT_BUILD=libm2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_PYTHON=OFF\
		-DENABLE_CSHARP=OFF\
		-DBUILD_EXAMPLES=OFF\
		-DENABLE_TOOLS=ON\
		-DINSTALL_UDEV_RULES=OFF\
		"
	__build_with_cmake
}

build_spdlog() {
	[ -f "/usr/bin/x86_64-w64-mingw32-windres.exe" ] && rm -v /usr/bin/x86_64-w64-mingw32-windres.exe
	ln -s /usr/bin/windres.exe /usr/bin/x86_64-w64-mingw32-windres.exe
	CURRENT_BUILD=spdlog
	CURRENT_BUILD_CMAKE_OPTS="\
		-DSPDLOG_BUILD_SHARED=ON\
		-DSPDLOG_BUILD_EXAMPLE=OFF\
		"
	__build_with_cmake
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
	__build_with_cmake
}

build_volk() {
	CURRENT_BUILD=volk
	CURRENT_BUILD_POST_CLEAN="git submodule update --init ../cpu_features"
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_MODTOOL=OFF\
		-DENABLE_TESTING=OFF\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	__build_with_cmake

}

build_gnuradio() {
	CURRENT_BUILD=gnuradio
	CURRENT_BUILD_CMAKE_OPTS="\
		-DENABLE_DEFAULT=OFF\
		-DENABLE_GNURADIO_RUNTIME=ON\
		-DENABLE_GR_ANALOG=ON\
		-DENABLE_GR_BLOCKS=ON\
		-DENABLE_GR_FFT=ON\
		-DENABLE_GR_FILTER=ON\
		-DENABLE_VOLK=ON\
		-DENABLE_GR_IIO=ON\
		-DCMAKE_C_FLAGS=-fno-asynchronous-unwind-tables\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	__build_with_cmake
}

build_grm2k() {
	CURRENT_BUILD=gr-m2k
	CURRENT_BUILD_CMAKE_OPTS="\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR==$STAGING_DIR/lib/python3.10/site-packages\
		"
	__build_with_cmake
}

build_grscopy() {
	CURRENT_BUILD=gr-scopy
	CURRENT_BUILD_CMAKE_OPTS="\
		-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
		-DGR_PYTHON_DIR=$STAGING_DIR/lib/python3.10/site-packages\
		"
	__build_with_cmake
}

build_qwt() {
	echo "### Building qwt - branch $QWT_BRANCH"
	CURRENT_BUILD=qwt
	pushd $WORKFOLDER/$CURRENT_BUILD
	git clean -xdf
	$QMAKE qwt.pro
	VERBOSE=1 make LDFLAGS="-Wl,verbose"  $JOBS
	make INSTALL_ROOT="$STAGING_DIR" $JOBS install
	cp $STAGING_DIR/lib/qwt.dll $STAGING_DIR/bin/qwt.dll
	echo "$CURRENT_BUILD - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd
}

build_libsigrokdecode() {
	echo "### Building libsigrokdecode - branch $LIBSIGROKDECODE_BRANCH"
	CURRENT_BUILD=libsigrokdecode

	pushd $WORKFOLDER/$CURRENT_BUILD
	if [ -d "$WORKFOLDER/libsigrokdecode/build-$ARCH" ]; then
		# hack .. this gets messed up somehow in docker due to changing files to lowercase
		git reset --hard
		git clean -xdf
	fi

	rm -rf ${WORKFOLDER}/libsigrokdecode/build-${ARCH}
	mkdir -p ${WORKFOLDER}/libsigrokdecode/build-${ARCH}
	cd ${WORKFOLDER}/libsigrokdecode

	patch -p1 < ${WORKFOLDER}/sigrokdecode-windows-fix.patch
	./autogen.sh
	cd build-${ARCH}

	CPPFLAGS="-DLIBSIGROKDECODE_EXPORT=1" ../configure ${AUTOCONF_OPTS}
	$MAKE_BIN $JOBS install
	rm -rf ${WORKFOLDER}/libsigrokdecode/build-${ARCH}
	echo "$CURRENT_BUILD - $(git rev-parse --short HEAD)" >> $BUILD_STATUS_FILE
	popd
}

build_libtinyiiod() {
	echo "### Building libtinyiiod - branch $LIBTINYIIOD_BRANCH"
	CURRENT_BUILD=libtinyiiod
	CURRENT_BUILD_CMAKE_OPTS="-DBUILD_EXAMPLES=OFF"
	__build_with_cmake
}

build_scopy() {
	CURRENT_BUILD=scopy
	NO_INSTALL="TRUE"
	CURRENT_BUILD_CMAKE_OPTS="$RC_COMPILER_OPT\
	-DBREAKPAD_HANDLER=ON\
	-DWITH_DOC=ON\
	-DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe\
	-DENABLE_PLUGIN_M2K=ON
	-DEMBED_PYTHON=ON
	"
	__build_with_cmake
}

write_status_file() {
	echo "" >> $BUILD_STATUS_FILE
	echo "$PACMAN -Qe output - all explicitly installed packages on build machine" >> $BUILD_STATUS_FILE
	$PACMAN -Qe >> $BUILD_STATUS_FILE
	echo "pacman -Qm output - all packages from nonsync sources" >> $BUILD_STATUS_FILE
	$PACMAN -Qm >> $BUILD_STATUS_FILE
	cp $BUILD_STATUS_FILE ${WORKFOLDER}/scopy-$MINGW_VERSION-build-status-file.txt
	echo -n ${PACMAN_SYNC_DEPS} > ${WORKFOLDER}/scopy-$MINGW_VERSION-build-deps-pacman.txt
}

build_deps() {
	build_libiio
	build_libad9361
	build_libm2k
	build_spdlog
	build_libsndfile
	build_volk
	build_gnuradio
	build_grscopy
	build_grm2k
	build_qwt
	build_libsigrokdecode
	build_libtinyiiod
}


for arg in $@; do
	$arg
done

#recurse_submodules
#install_tools
#install_deps
#create_build_status_file
#build_deps
#build_scopy # for testing
#package_and_push

#!/bin/bash
set -e

source ./CI/travis/lib.sh

__cmake() {
	local args="$1"
	mkdir -p build
	pushd build # build

	if [ "$TRAVIS" == "true" ] ; then
		cmake $args ..
		make -j${NUM_JOBS}
		sudo make install
	else
		cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
			-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" \
			$args .. $SILENCED
		CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS} $SILENCED
		make install
	fi

	popd
}

__make() {
	$preconfigure
	if [ "$TRAVIS" == "true" ] ; then
		$configure
		$make -j${NUM_JOBS}
		sudo $make install
	else
		$configure --prefix="$STAGINGDIR" $SILENCED
		CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib $make -j${NUM_JOBS} $SILENCED
		$SUDO $make install
	fi
}

__qmake() {
	if [ "$TRAVIS" == "true" ] ; then
		$QMAKE $qtarget
		make -j${NUM_JOBS}
		sudo make install
	else
		$QMAKE "$qtarget" $SILENCED
		QMAKE=$QMAKE CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib \
			make -j${NUM_JOBS} $SILENCED
		$SUDO make install
	fi
}

__build_common() {
	local dir="$1"
	local buildfunc="$2"
	local getfunc="$3"
	local subdir="$4"
	local args="$5"

	pushd "$WORKDIR" # deps dir

	# if we have this folder, we may not need to download it
	[ -d "$dir" ] || $getfunc

	pushd "$dir" # this dep dir
	[ -z "$subdir" ] || pushd "$subdir" # in case there is a build subdir or smth

	$buildfunc "$args"

	popd
	popd
	[ -z "$subdir" ] || popd
}

wget_and_untar() {
	[ -d "$WORKDIR/$dir" ] || {
		local tar_file="${dir}.tar.gz"
		wget --no-check-certificate "$url" -O "$tar_file"
		tar -xvf "$tar_file" > /dev/null
		[ -z "$patchfunc" ] || {
			pushd $dir
			$patchfunc
			popd
		}
	}
}

git_clone_update() {
	[ -d "$WORKDIR/$dir" ] || {
		[ -z "$branch" ] || branch="-b $branch"
		git clone --recursive $branch "$url" "$dir"
		[ -z "$patchfunc" ] || {
			pushd $dir
			$patchfunc
			popd
		}
	}
}

cmake_build_wget() {
	local dir="$1"
	local url="$2"

	__build_common "$dir" "__cmake" "wget_and_untar"
}

cmake_build_git() {
	local dir="$1"
	local url="$2"
	local branch="$3"
	local args="$4"

	__build_common "$dir" "__cmake" "git_clone_update" "" "$args"
}

make_build_wget() {
	local dir="$1"
	local url="$2"
	local configure="${3:-./configure}"
	local make="${4:-make}"

	__build_common "$dir" "__make" "wget_and_untar"
}

make_build_git() {
	local dir="$1"
	local url="$2"
	local configure="${3:-./configure}"
	local make="${4:-make}"
	local preconfigure="$5"

	__build_common "$dir" "__make" "git_clone_update"
}

qmake_build_wget() {
	local dir="$1"
	local url="$2"
	local qtarget="$3"
	local patchfunc="$4"

	__build_common "$dir" "__qmake" "wget_and_untar"
}

qmake_build_git() {
	local dir="$1"
	local url="$2"
	local branch="$3"
	local qtarget="$4"
	local patchfunc="$5"

	__build_common "$dir" "__qmake" "git_clone_update"
}

patch_qwt() {
	patch -p1 <<-EOF
--- a/qwtconfig.pri
+++ b/qwtconfig.pri
@@ -19,7 +19,7 @@ QWT_VERSION      = \$\${QWT_VER_MAJ}.\$\${QWT_VER_MIN}.\$\${QWT_VER_PAT}
 QWT_INSTALL_PREFIX = \$\$[QT_INSTALL_PREFIX]
 
 unix {
-    QWT_INSTALL_PREFIX    = /usr/local/qwt-\$\$QWT_VERSION-svn
+    QWT_INSTALL_PREFIX    = $STAGINGDIR
     # QWT_INSTALL_PREFIX = /usr/local/qwt-\$\$QWT_VERSION-svn-qt-\$\$QT_VERSION
 }
 
@@ -161,7 +161,7 @@ QWT_CONFIG     += QwtPlayground
 
 macx:!static:CONFIG(qt_framework, qt_framework|qt_no_framework) {
 
-    QWT_CONFIG += QwtFramework
+    #QWT_CONFIG += QwtFramework
 }  
 
 ######################################################################
--- a/src/src.pro
+++ b/src/src.pro
@@ -30,7 +30,8 @@ contains(QWT_CONFIG, QwtDll) {
     
         # we increase the SONAME for every minor number
 
-        QWT_SONAME=libqwt.so.\$\${VER_MAJ}.\$\${VER_MIN}
+        !macx: QWT_SONAME=libqwt.so.\$\${VER_MAJ}.\$\${VER_MIN}
+        macx: QWT_SONAME=\$\${QWT_INSTALL_LIBS}/libqwt.dylib
         QMAKE_LFLAGS *= \$\${QMAKE_LFLAGS_SONAME}\$\${QWT_SONAME}
         QMAKE_LFLAGS_SONAME=
     }
--- a/textengines/mathml/mathml.pro
+++ b/textengines/mathml/mathml.pro
@@ -57,7 +57,8 @@ contains(QWT_CONFIG, QwtDll) {
 
         # we increase the SONAME for every minor number
 
-        QWT_SONAME=libqwtmathml.so.\$\${VER_MAJ}.\$\${VER_MIN}
+        !macx: QWT_SONAME=libqwtmathml.so.\$\${VER_MAJ}.\$\${VER_MIN}
+        macx: QWT_SONAME=\$\${QWT_INSTALL_LIBS}/libqwtmathml.dylib
         QMAKE_LFLAGS *= \$\${QMAKE_LFLAGS_SONAME}\$\${QWT_SONAME}
         QMAKE_LFLAGS_SONAME=
     }   
EOF
}

patch_qwtpolar() {
	wget https://raw.githubusercontent.com/analogdevicesinc/scopy-flatpak/master/qwtpolar-qwt-6.1-compat.patch -O - | patch -p1

	patch -p1 <<-EOF
--- a/qwtpolarconfig.pri
+++ b/qwtpolarconfig.pri
@@ -70,14 +72,14 @@ QWT_POLAR_INSTALL_FEATURES  = \$\${QWT_POLAR_INSTALL_PREFIX}/features
 # Otherwise you have to build it from the designer directory.
 ######################################################################
 
-QWT_POLAR_CONFIG     += QwtPolarDesigner
+#QWT_POLAR_CONFIG     += QwtPolarDesigner
 
 ######################################################################
 # If you want to auto build the examples, enable the line below
 # Otherwise you have to build them from the examples directory.
 ######################################################################
 
-QWT_POLAR_CONFIG     += QwtPolarExamples
+#QWT_POLAR_CONFIG     += QwtPolarExamples
 
 ######################################################################
 # When Qt has been built as framework qmake wants 
@@ -86,6 +88,6 @@ QWT_POLAR_CONFIG     += QwtPolarExamples
 
 macx:CONFIG(qt_framework, qt_framework|qt_no_framework) {
 
-    QWT_POLAR_CONFIG += QwtPolarFramework
+    #QWT_POLAR_CONFIG += QwtPolarFramework
 }
 
EOF
}

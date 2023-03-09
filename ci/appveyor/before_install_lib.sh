#!/bin/bash
set -e


NUM_JOBS=4
if [ "$APPVEYOR" == "true" ] ; then
	STAGINGDIR=/usr/local
else
	STAGINGDIR="${WORKDIR}/staging"
fi

export PYTHON3=python3

__cmake() {
	local args="$1"
	mkdir -p build
	pushd build # build

	cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" \
		$args .. $SILENCED
	CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS} $SILENCED
	make -j${NUM_JOBS}
	sudo make install
	popd
}

__make() {
	$preconfigure
	$configure --prefix="$STAGINGDIR"
	$make -j${NUM_JOBS}
	sudo $make install
}

__qmake() {
	$QMAKE $qtarget
	make -j${NUM_JOBS}
	sudo make install
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

qmake_build_local() {
	local dir="$1"
	local qtarget="$2"
	local patchfunc="$3"
	[ ! -d "$WORKDIR/$dir" ] || {
		[ -z "$patchfunc" ] || {
			pushd $WORKDIR/$dir
			$patchfunc
			popd
		}
	}
	__build_common "$dir" "__qmake"
}

patch_qwt() {
	patch -p1 <<-EOF
--- a/qwtconfig.pri
+++ b/qwtconfig.pri
@@ -19,7 +19,7 @@
 QWT_INSTALL_PREFIX = \$\$[QT_INSTALL_PREFIX]
 
 unix {
-    QWT_INSTALL_PREFIX    = /usr/local
+    QWT_INSTALL_PREFIX    = $STAGINGDIR
     # QWT_INSTALL_PREFIX = /usr/local/qwt-\$\$QWT_VERSION-ma-qt-\$\$QT_VERSION
 }

@@ -42,7 +42,7 @@ QWT_INSTALL_LIBS      = \$\${QWT_INSTALL_PREFIX}/lib
 # runtime environment of designer/creator.
 ######################################################################
 
-QWT_INSTALL_PLUGINS   = \$\${QWT_INSTALL_PREFIX}/plugins/designer
+#QWT_INSTALL_PLUGINS   = \$\${QWT_INSTALL_PREFIX}/plugins/designer
 
 # linux distributors often organize the Qt installation
 # their way and QT_INSTALL_PREFIX doesn't offer a good
@@ -163,7 +163,7 @@ QWT_CONFIG     += QwtOpenGL
 
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
EOF
}


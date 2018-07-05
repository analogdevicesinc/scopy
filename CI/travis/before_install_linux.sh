#!/bin/bash
set -e

sudo apt-get -qq update
sudo apt-get install -y build-essential g++ bison flex cmake libxml2-dev \
	qt5-default qttools5-dev \
	qtdeclarative5-dev libqt5svg5-dev libglibmm-2.4-dev libmatio-dev libglib2.0-dev \
	libzip-dev libfftw3-dev libusb-dev doxygen libqt5opengl5-dev

source ./CI/travis/before_install_lib.sh

patch_qwtpolar_linux() {
	[ -f qwtpolar-qwt-6.1-compat.patch ] || {
		patch_qwtpolar

		patch -p1 <<-EOF
--- a/qwtpolarconfig.pri
+++ b/qwtpolarconfig.pri
@@ -16,7 +16,9 @@ QWT_POLAR_VER_PAT      = 1
 QWT_POLAR_VERSION      = \$\${QWT_POLAR_VER_MAJ}.\$\${QWT_POLAR_VER_MIN}.\$\${QWT_POLAR_VER_PAT}
 
 unix {
-    QWT_POLAR_INSTALL_PREFIX    = /usr/local/qwtpolar-\$\$QWT_POLAR_VERSION
+    QWT_POLAR_INSTALL_PREFIX    = $STAGINGDIR
+    QMAKE_CXXFLAGS              = -I${STAGINGDIR}/include
+    QMAKE_LFLAGS                = -L${STAGINGDIR}/lib
 }
 
 win32 {
EOF
	}
}

make_build_wget "boost_1_63_0" "https://netcologne.dl.sourceforge.net/project/boost/boost/1.63.0/boost_1_63_0.tar.gz" "./bootstrap.sh" "./b2"

make_build_git "libsigrok" "https://github.com/sigrokproject/libsigrok" "" "" "./autogen.sh"

make_build_wget "libsigrokdecode-0.4.1" "http://sigrok.org/download/source/libsigrokdecode/libsigrokdecode-0.4.1.tar.gz"

qmake_build_git "qwt" "https://github.com/osakared/qwt.git" "qwt-6.1-multiaxes" "qwt.pro" "patch_qwt"

qmake_build_wget "qwtpolar-1.1.1" "https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2" "qwtpolar.pro" "patch_qwtpolar_linux"

cmake_build_wget "volk-1.3" "http://libvolk.org/releases/volk-1.3.tar.gz"

cmake_build_git "gnuradio" "https://github.com/analogdevicesinc/gnuradio" "signal_source_phase_rebased" "-DENABLE_INTERNAL_VOLK:BOOL=OFF -DENABLE_GR_FEC:BOOL=OFF -DENABLE_GR_DIGITAL:BOOL=OFF -DENABLE_GR_DTV:BOOL=OFF -DENABLE_GR_ATSC:BOOL=OFF -DENABLE_GR_AUDIO:BOOL=OFF -DENABLE_GR_CHANNELS:BOOL=OFF -DENABLE_GR_NOAA:BOOL=OFF -DENABLE_GR_PAGER:BOOL=OFF -DENABLE_GR_TRELLIS:BOOL=OFF -DENABLE_GR_VOCODER:BOOL=OFF"

cmake_build_git "libiio" "https://github.com/analogdevicesinc/libiio" "" "-DINSTALL_UDEV_RULE:BOOL=OFF"

cmake_build_git "libad9361-iio" "https://github.com/analogdevicesinc/libad9361-iio"

cmake_build_git "gr-iio" "https://github.com/analogdevicesinc/gr-iio"

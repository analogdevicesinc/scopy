#!/bin/bash
set -e

handle_ubuntu_flatpak_docker() {
	sudo apt-get -qq update
	sudo service docker restart
	sudo docker pull alexandratr/ubuntu-flatpak-kde:latest
}

handle_ubuntu_docker() {
	sudo apt-get -qq update
	sudo service docker restart
	sudo docker pull ubuntu:${OS_VERSION}
}

handle_default() {
	pwd
	ls
	. /$LIBNAME/CI/travis/lib.sh


	if [ -z "${LDIST}" -a -f "build/.LDIST" ] ; then
		export LDIST="-$(cat build/.LDIST)"
	fi
	if [ -z "${LDIST}" ] ; then
		export LDIST="-$(get_ldist)"
	fi

	if ! is_new_ubuntu ; then
		CODENAME=-"$(get_codename)"
		sudo add-apt-repository --yes ppa:beineri/opt-qt592${CODENAME}
	fi

sudo apt-get -qq update
sudo apt-get install -y build-essential g++ bison flex libxml2-dev libglibmm-2.4-dev \
	libmatio-dev libglib2.0-dev libzip-dev libfftw3-dev libusb-dev doxygen \
	python-cheetah

# Trusty has an old boost and a newer boost; 1.55 is the minimum for Scopy
if [ "$(get_codename)" == "trusty" ] ; then
	sudo apt-get install -y cmake3
	BOOST_VER=1.55
else
	sudo apt-get install -y cmake
fi

BOOST_PACKAGES_BASE="libboost libboost-regex libboost-date-time
	libboost-program-options libboost-test libboost-filesystem
	libboost-system libboost-thread"

for package in $BOOST_PACKAGES_BASE ; do
	BOOST_PACKAGES="$BOOST_PACKAGES ${package}${BOOST_VER}-dev"
done

sudo apt-get install -y $BOOST_PACKAGES

. /$LIBNAME/CI/travis/before_install_lib.sh

if ! is_new_ubuntu ; then
	sudo apt-get install -y qt59base qt59declarative qt59quickcontrols \
		qt59svg qt59tools python-dev automake libtool mesa-common-dev \
		libegl1-mesa-dev libgl1-mesa-dev libgles2-mesa-dev libglu1-mesa-dev
	# temporarily disable `set -e`
	QMAKE=/opt/qt59/bin/qmake
	$QMAKE -set QMAKE $QMAKE
	set +e
	. /opt/qt59/bin/qt59-env.sh
	set -e
else
	sudo apt-get install -y qt5-default qttools5-dev qtdeclarative5-dev \
		libqt5svg5-dev libqt5opengl5-dev
	QMAKE="$(command -v qmake)"
fi

patch_qwtpolar_linux() {
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

if ! is_new_ubuntu ; then
	make_build_git "libsigrok" "https://github.com/sigrokproject/libsigrok" "" "" "./autogen.sh"

	make_build_wget "libsigrokdecode-0.4.1" "http://sigrok.org/download/source/libsigrokdecode/libsigrokdecode-0.4.1.tar.gz"

	cmake_build_wget "volk-1.3" "http://libvolk.org/releases/volk-1.3.tar.gz"
else
	sudo apt-get install -y \
		libvolk1-dev libsigrok-dev libsigrokcxx-dev libsigrokdecode-dev
fi

qmake_build_git "qwt" "https://github.com/osakared/qwt.git" "qwt-6.1-multiaxes" "qwt.pro" "patch_qwt"

qmake_build_wget "qwtpolar-1.1.1" "https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2" "qwtpolar.pro" "patch_qwtpolar_linux"

cmake_build_git "gnuradio" "https://github.com/analogdevicesinc/gnuradio" "scopy" "-DENABLE_INTERNAL_VOLK:BOOL=OFF -DENABLE_GR_FEC:BOOL=OFF -DENABLE_GR_DIGITAL:BOOL=OFF -DENABLE_GR_DTV:BOOL=OFF -DENABLE_GR_ATSC:BOOL=OFF -DENABLE_GR_AUDIO:BOOL=OFF -DENABLE_GR_CHANNELS:BOOL=OFF -DENABLE_GR_NOAA:BOOL=OFF -DENABLE_GR_PAGER:BOOL=OFF -DENABLE_GR_TRELLIS:BOOL=OFF -DENABLE_GR_VOCODER:BOOL=OFF"

if [ "$TRAVIS" == "true" ] ; then
#	for pkg in libiio libad9361-iio ; do
#		wget http://swdownloads.analog.com/cse/travis_builds/master_latest_${pkg}${LDIST}.deb
#		sudo dpkg -i ./master_latest_${pkg}${LDIST}.deb
#	done
	wget http://swdownloads.analog.com/cse/travis_builds/master_latest_libiio${LDIST}.deb
	sudo dpkg -i ./master_latest_libiio${LDIST}.deb
	wget http://swdownloads.analog.com/cse/travis_builds/master_latest_libad9361-iio${CODENAME}.deb
	sudo dpkg -i ./master_latest_libad9361-iio${CODENAME}.deb
else
	cmake_build_git "libiio" "https://github.com/analogdevicesinc/libiio" "" "-DINSTALL_UDEV_RULE:BOOL=OFF"

	cmake_build_git "libad9361-iio" "https://github.com/analogdevicesinc/libad9361-iio" "" "-DLIBIIO_INCLUDEDIR:PATH=$STAGINGDIR/include -DLIBIIO_LIBRARIES:FILEPATH=$STAGINGDIR/lib/libiio.so"
fi

cmake_build_git "gr-iio" "https://github.com/analogdevicesinc/gr-iio"
}

OS_TYPE=${1:-default}
OS_VERSION=${2}
LIBNAME=${3:-home/travis/build/analogdevicesinc/scopy}

handle_${OS_TYPE}

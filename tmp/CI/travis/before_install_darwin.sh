#!/bin/sh
set -e

. CI/travis/lib.sh

if [ -z "${LDIST}" -a -f "build/.LDIST" ] ; then
	export LDIST="-$(cat build/.LDIST)"
fi
if [ -z "${LDIST}" ] ; then
	export LDIST="-$(get_ldist)"
fi

brew update

brew_install_or_upgrade() {
	brew install $1 || \
		brew upgrade $1 || \
		brew ls --versions $1 # check if installed last-ly
}

brew_install() {
	brew install $1 || \
		brew ls --versions $1
}

PYTHON="python3 python@2 python brew-pip"
PACKAGES="qt cmake fftw bison gettext autoconf automake libtool libzip glib libusb $PYTHON"
PACKAGES="$PACKAGES glibmm doxygen wget boost gnu-sed libmatio dylibbundler libxml2 pkg-config"

for pak in $PACKAGES ; do
	brew_install $pak
done

brew upgrade python3

for pkg in qt gcc bison gettext; do
	brew link --overwrite --force $pkg
done

. CI/travis/before_install_lib.sh

QT_PATH="$(brew --prefix qt)/bin"
export PATH="${QT_PATH}:$PATH"
export PATH="/usr/local/opt/bison/bin:$PATH"

patch_qwtpolar_mac() {
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
+    QMAKE_LFLAGS                = ${STAGINGDIR}/lib/libqwt*dylib
 }
 
 win32 {
EOF
}

# Get pip if not installed ; on Travis + OS X, Python is not well supported
if ! command -v pip ; then
	curl https://bootstrap.pypa.io/get-pip.py > get-pip.py
	sudo -H python get-pip.py
fi

QMAKE="$(command -v qmake)"

pip install --user cheetah

make_build_git "libsigrok" "https://github.com/sigrokproject/libsigrok" "" "" "./autogen.sh"

make_build_wget "libsigrokdecode-0.4.1" "http://sigrok.org/download/source/libsigrokdecode/libsigrokdecode-0.4.1.tar.gz"

qmake_build_git "qwt" "https://github.com/osakared/qwt.git" "qwt-6.1-multiaxes" "qwt.pro" "patch_qwt"

qmake_build_wget "qwtpolar-1.1.1" "https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2" "qwtpolar.pro" "patch_qwtpolar_mac"

cmake_build_wget "volk-1.3" "http://libvolk.org/releases/volk-1.3.tar.gz"

cmake_build_git "gnuradio" "https://github.com/analogdevicesinc/gnuradio" "scopy" "-DENABLE_INTERNAL_VOLK:BOOL=OFF -DENABLE_GR_FEC:BOOL=OFF -DENABLE_GR_DIGITAL:BOOL=OFF -DENABLE_GR_DTV:BOOL=OFF -DENABLE_GR_ATSC:BOOL=OFF -DENABLE_GR_AUDIO:BOOL=OFF -DENABLE_GR_CHANNELS:BOOL=OFF -DENABLE_GR_NOAA:BOOL=OFF -DENABLE_GR_PAGER:BOOL=OFF -DENABLE_GR_TRELLIS:BOOL=OFF -DENABLE_GR_VOCODER:BOOL=OFF"

if [ "$TRAVIS" == "true" ] ; then
	for pkg in libiio libad9361-iio ; do
		wget http://swdownloads.analog.com/cse/travis_builds/master_latest_${pkg}${LDIST}.pkg
		sudo installer -pkg master_latest_${pkg}${LDIST}.pkg -target /
	done
else
	SUDO=sudo
	cmake_build_git "libiio" "https://github.com/analogdevicesinc/libiio" "" "-DINSTALL_UDEV_RULE:BOOL=OFF"

	cmake_build_git "libad9361-iio" "https://github.com/analogdevicesinc/libad9361-iio"
	# no longer need sudo from here
	unset SUDO
fi

cmake_build_git "gr-iio" "https://github.com/analogdevicesinc/gr-iio"

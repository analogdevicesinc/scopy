#!/bin/sh
set -e

brew update
# Install Qt
brew install qt
echo 'export PATH="$(brew --prefix qt)/bin:$PATH"' >> ~/.bash_profile
brew link --force qt

brew_install_or_upgrade() {
	brew install $1 || \
		brew upgrade $1 || \
		brew ls --versions $1 # check if installed last-ly
}

PACKAGES="cmake fftw bison gettext autoconf automake libtool libzip glib libusb python3 python brew-pip"
PACKAGES="$PACKAGES glibmm doxygen wget boost gnu-sed libmatio dylibbundler libxml2 pkg-config"

for pak in $PACKAGES ; do
	brew_install_or_upgrade $pak
done

brew link --overwrite --force gcc
brew link --overwrite --force bison
brew link --overwrite --force gettext

source ./CI/travis/before_install_lib.sh

# Get pip if not installed ; on Travis + OS X, Python is not well supported
if ! command -v pip ; then
	curl https://bootstrap.pypa.io/get-pip.py > get-pip.py
	sudo -H python get-pip.py
fi

pip install --user cheetah

make_build_git "libsigrok" "https://github.com/sigrokproject/libsigrok" "" "" "./autogen.sh"

make_build_wget "libsigrokdecode-0.4.1" "http://sigrok.org/download/source/libsigrokdecode/libsigrokdecode-0.4.1.tar.gz"

qmake_build_git "qwt" "https://github.com/osakared/qwt.git" "qwt-6.1-multiaxes" "qwt.pro" "patch_qwt"

qmake_build_wget "qwtpolar-1.1.1" "https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2" "qwtpolar.pro" "patch_qwtpolar"

cmake_build_wget "volk-1.3" "http://libvolk.org/releases/volk-1.3.tar.gz"

cmake_build_git "gnuradio" "https://github.com/analogdevicesinc/gnuradio" "signal_source_phase" "-DENABLE_INTERNAL_VOLK:BOOL=OFF -DENABLE_GR_FEC:BOOL=OFF -DENABLE_GR_DIGITAL:BOOL=OFF -DENABLE_GR_DTV:BOOL=OFF -DENABLE_GR_ATSC:BOOL=OFF -DENABLE_GR_AUDIO:BOOL=OFF -DENABLE_GR_CHANNELS:BOOL=OFF -DENABLE_GR_NOAA:BOOL=OFF -DENABLE_GR_PAGER:BOOL=OFF -DENABLE_GR_TRELLIS:BOOL=OFF -DENABLE_GR_VOCODER:BOOL=OFF"

cmake_build_git "libiio" "https://github.com/analogdevicesinc/libiio" "" "-DINSTALL_UDEV_RULE:BOOL=OFF"

cmake_build_git "libad9361-iio" "https://github.com/analogdevicesinc/libad9361-iio"

cmake_build_git "gr-iio" "https://github.com/analogdevicesinc/gr-iio"

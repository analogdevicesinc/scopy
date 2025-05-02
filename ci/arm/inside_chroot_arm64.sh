#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
ln -snf /usr/share/zoneinfo/Europe/Bucharest /etc/localtime && echo "Europe/Bucharest" > /etc/timezone
echo "LC_ALL=en_US.UTF-8" | tee -a /etc/environment
echo "LANG=en_US.UTF-8" | tee -a /etc/locale.conf
locale-gen en_US.UTF-8

apt -y remove *qt*
apt -y autoremove

rm -rfv $(find / | grep libiio)
rm -rfv $(find / | grep m2k)
rm -rfv $(find / | grep libad9361)
rm -rfv $(find / | grep libad9166)

apt update && apt -y upgrade
apt -y dist-upgrade
dpkg --configure -a

apt -y install '^libxcb.*-dev' autoconf automake bison build-essential cmake dh-python figlet flex freeglut3-dev g++ gawk gcc \
	gdb-multiarch gdbserver git libavahi-client* libavahi-common* libboost1.81-all-dev libdrm-dev libgbm-dev libglib2.0-dev libgl1-mesa-dev libgles2-mesa-dev libglu1-mesa-dev \
	libgmp-dev libinput-dev libopenal-dev libsndfile1-dev libspeechd-dev libts-dev libudev-dev libunwind-dev libxcb-icccm4 libxcb-xinerama0 \
	libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrender-dev libxml2-dev libxml2-utils mesa-common-dev mesa-utils* \
	perl pkg-config unzip wget libmatio-dev \
	libusb-1.0-0-dev libaio-dev libpkgconf-dev libpython3-all-dev libfftw3-dev swig swig4.0 libgsl-dev libfuse2  libzmq3-dev libwayland-egl-backend-dev \
	libwayland-dev libwayland-egl1-mesa libssl-dev \
	libjpeg-dev libpng-dev libxcomposite-dev libdouble-conversion-dev \
	libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-shape0-dev libxcb-xinerama0 libxcb-xkb1 libxkbcommon-x11-0

apt -y remove *qt*

wget https://raw.githubusercontent.com/abhiTronix/raspberry-pi-cross-compilers/master/utils/SSymlinker
sed -i 's/sudo//g' SSymlinker
chmod +x SSymlinker
./SSymlinker -s /usr/include/aarch64-linux-gnu/asm -d /usr/include
./SSymlinker -s /usr/include/aarch64-linux-gnu/gnu -d /usr/include
./SSymlinker -s /usr/include/aarch64-linux-gnu/bits -d /usr/include
./SSymlinker -s /usr/include/aarch64-linux-gnu/sys -d /usr/include
./SSymlinker -s /usr/lib/aarch64-linux-gnu/crtn.o -d /usr/lib/crtn.o
./SSymlinker -s /usr/lib/aarch64-linux-gnu/crt1.o -d /usr/lib/crt1.o
./SSymlinker -s /usr/lib/aarch64-linux-gnu/crti.o -d /usr/lib/crti.o

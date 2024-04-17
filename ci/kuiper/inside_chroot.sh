#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
ln -snf /usr/share/zoneinfo/Europe/Bucharest /etc/localtime && echo "Europe/Bucharest" > /etc/timezone
echo "LC_ALL=en_US.UTF-8" | tee -a /etc/environment
echo "LANG=en_US.UTF-8" | tee -a /etc/locale.conf
locale-gen en_US.UTF-8

sed -i 's/#deb-src/deb-src/' /etc/apt/sources.list

apt -y purge openjdk* tex-common
apt -y remove gnuradio gnuradio-* libgnuradio-* libvolk*
apt -y remove qt* libqt5*
apt -y autoremove
apt update && apt -y upgrade
apt -y dist-upgrade
dpkg --configure -a

rm -rf /usr/local/include/volk /usr/local/lib/libvolk* \
	/usr/local/lib/cmake/volk
rm -rf /usr/lib/arm-linux-gnueabihf/libiio.so* \
	usr/local/src/libiio \
	/usr/lib/arm-linux-gnueabihf/pkgconfig/libiio.pc
rm -rf /usr/local/lib/cmake/m2k \
	/usr/local/lib/arm-linux-gnueabihf/pkgconfig/libm2k.pc \
	/usr/local/lib/arm-linux-gnueabihf/pkgconfig/gnuradio-m2k.pc \
	/usr/local/lib/arm-linux-gnueabihf/libgnuradio-m2k.so* \
	/usr/local/lib/arm-linux-gnueabihf/libm2k.so* \
	/usr/local/lib/arm-linux-gnueabihf/cmake/libm2k \
	/usr/local/include/libm2k /usr/local/include/m2k
rm -rf /usr/local/lib/arm-linux-gnueabihf/libgnuradio-iio.so* \
	/usr/local/lib/arm-linux-gnueabihf/pkgconfig/gnuradio-iio.pc \
	/usr/local/lib/cmake/iio
rm -rf /usr/lib/arm-linux-gnueabihf/libad9361.so* \
	/usr/lib/arm-linux-gnueabihf/pkgconfig/libad9361.pc \
	/usr/lib/libad9166.so* \
	/usr/lib/pkgconfig/libad9166.pc

apt -y build-dep qtbase5-dev || true
apt -y install build-essential gcc g++ gdb-multiarch cmake autoconf automake bison flex git wget pkg-config figlet gawk unzip libsndfile1-dev \
	libudev-dev libinput-dev libts-dev libxcb-xinerama0-dev libxcb-xinerama0 gdbserver libspeechd-dev perl \
	libgl1-mesa-dev libxcb-composite0-dev libxcb-cursor-dev libxcb-damage0-dev libxcb-xv0-dev \
	libxcb-dpms0-dev libxcb-dri2-0-dev libxcb-ewmh-dev libxcb-imdkit-dev libxcb-xvmc0-dev \
	libxcb-present-dev libxcb-record0-dev libxcb-res0-dev libxcb-xrm-dev libx11-xcb-dev libxcb-glx0-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-xkb-dev libxkbcommon-x11-dev \
	libxcb-screensaver0-dev libxcb-util0-dev libxcb-xf86dri0-dev libxcb-xtest0-dev
apt -y install libunwind-dev libsndfile1-dev mesa-utils* mesa-common-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev python2 libopenal-dev || true
apt -y install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev || true

wget https://raw.githubusercontent.com/abhiTronix/raspberry-pi-cross-compilers/master/utils/SSymlinker
sed -i 's/sudo//g' SSymlinker
chmod +x SSymlinker
./SSymlinker -s /usr/include/arm-linux-gnueabihf/asm -d /usr/include
./SSymlinker -s /usr/include/arm-linux-gnueabihf/gnu -d /usr/include
./SSymlinker -s /usr/include/arm-linux-gnueabihf/bits -d /usr/include
./SSymlinker -s /usr/include/arm-linux-gnueabihf/sys -d /usr/include
./SSymlinker -s /usr/lib/arm-linux-gnueabihf/crtn.o -d /usr/lib/crtn.o
./SSymlinker -s /usr/lib/arm-linux-gnueabihf/crt1.o -d /usr/lib/crt1.o
./SSymlinker -s /usr/lib/arm-linux-gnueabihf/crti.o -d /usr/lib/crti.o
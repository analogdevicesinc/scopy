#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
ln -snf /usr/share/zoneinfo/Europe/Bucharest /etc/localtime && echo "Europe/Bucharest" > /etc/timezone
echo "LC_ALL=en_US.UTF-8" | tee -a /etc/environment
echo "LANG=en_US.UTF-8" | tee -a /etc/locale.conf
locale-gen en_US.UTF-8

sed -i 's/#deb-src/deb-src/' /etc/apt/sources.list

apt -y purge openjdk* tex-common
apt -y remove gnuradio gnuradio-* libgnuradio-* libvolk*
apt -y remove *qt*
apt -y autoremove
apt update && apt -y upgrade
apt -y dist-upgrade
dpkg --configure -a

rm -rfv $(find / | grep libiio)
rm -rfv $(find / | grep m2k)
rm -rfv $(find / | grep libad9361)
rm -rfv $(find / | grep libad9166)
rm -rfv $(find / | grep volk)
rm -rfv $(find / | grep gnuradio)

apt -y build-dep qtbase5-dev
apt -y install '^libxcb.*-dev' autoconf automake bison build-essential cmake figlet flex freeglut3-dev g++ gawk gcc \
	gdb-multiarch gdbserver git libavahi-client* libavahi-common* libboost1.74-all-dev libdrm-dev libgbm-dev libglib2.0-dev libgl1-mesa-dev libgles2-mesa-dev \
	libglu1-mesa-dev libgmp-dev libinput-dev libopenal-dev libsndfile1-dev libspeechd-dev libts-dev libudev-dev \
	libunwind-dev libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrender-dev libxml2-dev libxml2-utils \
	mesa-common-dev mesa-utils* perl pkg-config dh-python unzip wget

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
#!/bin/bash

# ARM32 (armhf) Sysroot Configuration Script
# ==========================================
# Purpose: Configure the ARM sysroot for cross-compilation
# This script runs inside chroot with QEMU emulation

# Configure non-interactive package installation
export DEBIAN_FRONTEND=noninteractive

# Set timezone and locale
ln -snf /usr/share/zoneinfo/Europe/Bucharest /etc/localtime && echo "Europe/Bucharest" > /etc/timezone
echo "LC_ALL=en_US.UTF-8" | tee -a /etc/environment
echo "LANG=en_US.UTF-8" | tee -a /etc/locale.conf
locale-gen en_US.UTF-8

# Enable source repositories (needed for build-dep)
sed -i 's/#deb-src/deb-src/' /etc/apt/sources.list

# Update system packages
apt update && apt -y upgrade
apt -y dist-upgrade

# Remove unnecessary packages
apt -y purge openjdk* tex-common

# Remove pre-installed GNU Radio and Qt
apt -y remove gnuradio gnuradio-* libgnuradio-* libvolk*
apt -y remove *qt*

apt -y autoremove

# Fix any broken packages
dpkg --configure -a

# Remove any leftover files from ADI libraries
# These will be built from source with correct versions
find / -name '*libiio*' -exec rm -rfv {} +
find / -name '*m2k*' -exec rm -rfv {} +
find / -name '*libad9361*' -exec rm -rfv {} +
find / -name '*volk*' -exec rm -rfv {} +
find / -name '*gnuradio*' -exec rm -rfv {} +

# Install Qt build dependencies
apt -y build-dep qtbase5-dev

# Install development packages needed for building
apt -y install '^libxcb.*-dev' autoconf automake bison build-essential cmake figlet flex freeglut3-dev g++ gawk gcc \
	gdb-multiarch gdbserver git libavahi-client* libavahi-common* libboost1.74-all-dev libdrm-dev libgbm-dev libglib2.0-dev libgl1-mesa-dev libgles2-mesa-dev \
	libglu1-mesa-dev libgmp-dev libinput-dev libopenal-dev libsndfile1-dev libspeechd-dev libts-dev libudev-dev \
	libunwind-dev libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrender-dev libxml2-dev libxml2-utils \
	mesa-common-dev mesa-utils* perl pkg-config dh-python unzip wget libmatio-dev
apt -y install libusb-1.0-0-dev libaio-dev libpkgconf-dev libpython3-all-dev libfftw3-dev swig swig4.0 libgsl-dev libfuse2  libzmq3-dev libwayland-egl-backend-dev  libwayland-dev libwayland-egl1-mesa libssl-dev

# Remove any Qt packages that got pulled in
# We build Qt from source
apt -y remove *qt*

# Download symlink creation tool
wget https://raw.githubusercontent.com/abhiTronix/raspberry-pi-cross-compilers/master/utils/SSymlinker
# Remove sudo calls (we're already root in chroot)
sed -i 's/sudo//g' SSymlinker
chmod +x SSymlinker

# Create symlinks for system headers
./SSymlinker -s /usr/include/arm-linux-gnueabihf/asm -d /usr/include
./SSymlinker -s /usr/include/arm-linux-gnueabihf/gnu -d /usr/include
./SSymlinker -s /usr/include/arm-linux-gnueabihf/bits -d /usr/include
./SSymlinker -s /usr/include/arm-linux-gnueabihf/sys -d /usr/include

# Create symlinks for C runtime objects
./SSymlinker -s /usr/lib/arm-linux-gnueabihf/crtn.o -d /usr/lib/crtn.o
./SSymlinker -s /usr/lib/arm-linux-gnueabihf/crt1.o -d /usr/lib/crt1.o
./SSymlinker -s /usr/lib/arm-linux-gnueabihf/crti.o -d /usr/lib/crti.o
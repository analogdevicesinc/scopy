#!/bin/bash
set -e

# ARMHF Sysroot Configuration Script for Qt6
# ============================================
# Runs inside chroot with QEMU emulation
# Installs all development packages needed for cross-compiling Qt6 and Scopy
#
# Package groups are split to catch failures early (D13)

export DEBIAN_FRONTEND=noninteractive

ln -snf /usr/share/zoneinfo/Europe/Bucharest /etc/localtime && echo "Europe/Bucharest" > /etc/timezone

apt-get update && apt-get -y upgrade
apt-get -y dist-upgrade

# Remove pre-installed Qt and ADI packages — will be rebuilt from source
apt-get -y purge *qt* 2>/dev/null || true
apt-get -y autoremove || true

dpkg --configure -a

find / -path /proc -prune -o -name '*libiio*' -print -exec rm -rfv {} + 2>/dev/null || true
find / -path /proc -prune -o -name '*libm2k*' -print -exec rm -rfv {} + 2>/dev/null || true
find / -path /proc -prune -o -name '*libad9361*' -print -exec rm -rfv {} + 2>/dev/null || true
find / -path /proc -prune -o -name '*libad9166*' -print -exec rm -rfv {} + 2>/dev/null || true
find / -path /proc -prune -o -name '*volk*' -print -exec rm -rfv {} + 2>/dev/null || true
find / -path /proc -prune -o -name '*gnuradio*' -print -exec rm -rfv {} + 2>/dev/null || true

echo "=== Installing build tools ==="
apt-get -y install build-essential cmake pkg-config autoconf automake libtool

echo "=== Installing XCB libraries ==="
apt-get -y install \
	libxcb1-dev libxcb-xinerama0-dev libxcb-icccm4-dev libxcb-keysyms1-dev \
	libxcb-shape0-dev libxcb-xkb-dev libxcb-cursor-dev libxcb-image0-dev \
	libxcb-render-util0-dev libxcb-sync-dev libxcb-randr0-dev libxcb-shm0-dev \
	libxcb-util-dev libxcb-ewmh-dev libxcb-xfixes0-dev libxcb-render0-dev

echo "=== Installing X11 libraries ==="
apt-get -y install \
	libx11-xcb-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev \
	libxrender-dev libxcomposite-dev

echo "=== Installing GL/EGL libraries ==="
apt-get -y install \
	libgl-dev libegl-dev libgles-dev \
	libgl1-mesa-dev libgles2-mesa-dev libglu1-mesa-dev libegl1-mesa-dev

echo "=== Installing system libraries ==="
apt-get -y install \
	libdrm-dev libgbm-dev libinput-dev libudev-dev \
	libmtdev-dev libts-dev libevdev-dev libsystemd-dev libbrotli-dev

echo "=== Installing core libraries ==="
apt-get -y install \
	libglib2.0-dev libssl-dev libjpeg-dev libpng-dev \
	libdouble-conversion-dev libpcre2-dev libharfbuzz-dev \
	libfontconfig1-dev libfreetype-dev \
	zlib1g-dev libzstd-dev libbz2-dev liblzma-dev \
	libffi-dev libmd-dev libbsd-dev uuid-dev \
	libdbus-1-dev libsqlite3-dev libexpat1-dev \
	libicu-dev

echo "=== Installing Boost libraries ==="
apt-get -y install \
	libboost-system-dev libboost-filesystem-dev libboost-thread-dev libboost-chrono-dev \
	libboost-date-time-dev libboost-program-options-dev libboost-regex-dev libboost-test-dev

echo "=== Installing Scopy dependency libraries ==="
apt-get -y install \
	libfftw3-dev libgmp-dev libzip-dev \
	libmatio-dev liborc-0.4-dev \
	libavahi-client-dev libavahi-common-dev \
	libusb-1.0-0-dev libaio-dev libsndfile1-dev \
	libncurses-dev libzmq3-dev libxml2-dev \
	libwayland-dev libwayland-egl-backend-dev \
	libgsl-dev libfuse2t64 \
	python3-dev libreadline-dev \
	libgdbm-dev libdb5.3-dev

echo "=== Installing audio codec libraries (libsndfile deps) ==="
apt-get -y install \
	libflac-dev libvorbis-dev libogg-dev libopus-dev \
	libmpg123-dev libmp3lame-dev

# Remove any Qt packages that got pulled in
apt-get -y remove *qt* 2>/dev/null || true

echo "=== Creating cross-compilation symlinks ==="
wget -q https://raw.githubusercontent.com/abhiTronix/raspberry-pi-cross-compilers/master/utils/SSymlinker -O /tmp/SSymlinker
sed -i 's/sudo//g' /tmp/SSymlinker
chmod +x /tmp/SSymlinker

/tmp/SSymlinker -s /usr/include/arm-linux-gnueabihf/asm -d /usr/include
/tmp/SSymlinker -s /usr/include/arm-linux-gnueabihf/gnu -d /usr/include
/tmp/SSymlinker -s /usr/include/arm-linux-gnueabihf/bits -d /usr/include
/tmp/SSymlinker -s /usr/include/arm-linux-gnueabihf/sys -d /usr/include

/tmp/SSymlinker -s /usr/lib/arm-linux-gnueabihf/crtn.o -d /usr/lib/crtn.o
/tmp/SSymlinker -s /usr/lib/arm-linux-gnueabihf/crt1.o -d /usr/lib/crt1.o
/tmp/SSymlinker -s /usr/lib/arm-linux-gnueabihf/crti.o -d /usr/lib/crti.o

rm /tmp/SSymlinker
echo "=== Sysroot configuration complete ==="

#!/bin/bash
# Verify host x86_64 packages required for armhf cross-compilation
# These are installed by: ./arm_cross_build_process.sh install_packages
# Usage: ./verify_host_packages.sh

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

PASS=0
FAIL=0

ok()   { echo -e "  ${GREEN}OK${NC}: $1"; PASS=$((PASS+1)); }
fail() { echo -e "  ${RED}MISSING${NC}: $1"; FAIL=$((FAIL+1)); }

check_dpkg() {
	local pkg="$1"
	local label="${2:-$pkg}"
	dpkg -s "$pkg" >/dev/null 2>&1 && ok "$label ($pkg)" || fail "$label ($pkg)"
}

check_cmd() {
	local cmd="$1"
	local label="${2:-$cmd}"
	command -v "$cmd" >/dev/null 2>&1 && ok "$label ($cmd)" || fail "$label ($cmd)"
}

echo "Verifying host packages for armhf cross-compilation"
echo ""

echo "============================================"
echo "1. Build tools"
echo "============================================"
check_dpkg build-essential "C/C++ compiler suite"
check_dpkg cmake "CMake"
check_dpkg git "Git"
check_dpkg wget "wget"
check_dpkg ca-certificates "CA certificates"
check_dpkg unzip "unzip"
check_dpkg ninja-build "Ninja build"
check_dpkg patchelf "patchelf"
check_dpkg squashfs-tools "squashfs-tools (mksquashfs)"

echo ""
echo "============================================"
echo "2. Autotools / code generation"
echo "============================================"
check_dpkg pkg-config "pkg-config"
check_dpkg flex "Flex lexer"
check_dpkg bison "Bison parser"
check_dpkg autogen "autogen"
check_dpkg autoconf "autoconf"
check_dpkg autoconf-archive "autoconf-archive"
check_dpkg libtool "libtool"
check_dpkg swig "SWIG"

echo ""
echo "============================================"
echo "3. Cross-compiler"
echo "============================================"
check_dpkg crossbuild-essential-armhf "armhf cross-compiler"
check_cmd arm-linux-gnueabihf-gcc "armhf gcc"
check_cmd arm-linux-gnueabihf-g++ "armhf g++"
check_cmd arm-linux-gnueabihf-ld "armhf linker"
check_cmd arm-linux-gnueabihf-ar "armhf ar"
check_cmd arm-linux-gnueabihf-strip "armhf strip"

echo ""
echo "============================================"
echo "4. Python (host)"
echo "============================================"
check_dpkg python3 "Python 3"
check_dpkg python3-pip "pip"
check_dpkg python3-dev "Python dev headers"
check_dpkg python3-numpy "numpy"
check_dpkg python3-packaging "packaging"
check_dpkg python3-mako "Mako templates"

echo ""
echo "============================================"
echo "5. GL/EGL (host — for Qt6 host build)"
echo "============================================"
check_dpkg libgl1-mesa-dev "Mesa GL"
check_dpkg libglu1-mesa-dev "Mesa GLU"
check_dpkg libegl1-mesa-dev "Mesa EGL"
check_dpkg libgles2-mesa-dev "Mesa GLESv2"

echo ""
echo "============================================"
echo "6. XCB (host — for Qt6 host build)"
echo "============================================"
check_dpkg libxcb1-dev "xcb core"
check_dpkg libxcb-xinerama0-dev "xcb xinerama"
check_dpkg libxcb-icccm4-dev "xcb icccm"
check_dpkg libxcb-xfixes0-dev "xcb xfixes"
check_dpkg libxcb-render0-dev "xcb render"
check_dpkg libxcb-keysyms1-dev "xcb keysyms"
check_dpkg libxcb-shape0-dev "xcb shape"
check_dpkg libxcb-xkb-dev "xcb xkb"
check_dpkg libxcb-cursor-dev "xcb cursor"
check_dpkg libxcb-image0-dev "xcb image"
check_dpkg libxcb-render-util0-dev "xcb render-util"
check_dpkg libxcb-sync-dev "xcb sync"
check_dpkg libxcb-randr0-dev "xcb randr"
check_dpkg libxcb-shm0-dev "xcb shm"
check_dpkg libxcb-util-dev "xcb util"
check_dpkg libxcb-ewmh-dev "xcb ewmh"

echo ""
echo "============================================"
echo "7. X11 / input (host — for Qt6 host build)"
echo "============================================"
check_dpkg libx11-xcb-dev "X11-xcb"
check_dpkg libxi-dev "Xi"
check_dpkg libxkbcommon-dev "xkbcommon"
check_dpkg libxkbcommon-x11-dev "xkbcommon-x11"
check_dpkg libxrender-dev "Xrender"
check_dpkg libxcomposite-dev "Xcomposite"
check_dpkg libdrm-dev "drm"
check_dpkg libgbm-dev "gbm"
check_dpkg libinput-dev "libinput"
check_dpkg libudev-dev "udev"

echo ""
echo "============================================"
echo "8. Core libraries (host — for Qt6 host build)"
echo "============================================"
check_dpkg libssl-dev "OpenSSL"
check_dpkg libjpeg-dev "JPEG"
check_dpkg libpng-dev "PNG"
check_dpkg libdouble-conversion-dev "double-conversion"
check_dpkg libglib2.0-dev "GLib"
check_dpkg libzstd-dev "zstd"
check_dpkg libbz2-dev "bzip2"
check_dpkg liblzma-dev "lzma"
check_dpkg libfontconfig1-dev "fontconfig"
check_dpkg libfreetype-dev "freetype"
check_dpkg libpcre2-dev "pcre2"
check_dpkg libharfbuzz-dev "harfbuzz"

echo ""
echo "============================================"
echo "9. CMake (custom, downloaded by build script)"
echo "============================================"
SRC_SCRIPT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
CMAKE_BIN=$SRC_SCRIPT/staging/cmake/bin/cmake
if [ -x "$CMAKE_BIN" ]; then
	VERSION=$($CMAKE_BIN --version 2>/dev/null | head -1)
	ok "custom cmake: $VERSION"
else
	fail "custom cmake at $CMAKE_BIN (run: ./arm_cross_build_process.sh download_cmake)"
fi

echo ""
echo "============================================"
echo "SUMMARY"
echo "============================================"
echo -e "  ${GREEN}Passed${NC}: $PASS"
echo -e "  ${RED}Failed${NC}: $FAIL"
echo ""

if [ $FAIL -gt 0 ]; then
	echo -e "${RED}HOST INCOMPLETE — $FAIL missing items${NC}"
	echo "Run: ./arm_cross_build_process.sh install_packages"
	exit 1
else
	echo -e "${GREEN}HOST OK${NC}"
	exit 0
fi

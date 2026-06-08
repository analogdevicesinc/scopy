#!/bin/bash
# Verify sysroot has all required libraries and headers for Scopy cross-compilation
# Usage: ./verify_sysroot.sh [sysroot_path]
#        ./verify_sysroot.sh --tarball path/to/sysroot-armhf.tar.gz

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0
WARN=0

ok()   { echo -e "  ${GREEN}OK${NC}: $1"; PASS=$((PASS+1)); }
fail() { echo -e "  ${RED}MISSING${NC}: $1"; FAIL=$((FAIL+1)); }
warn() { echo -e "  ${YELLOW}WARN${NC}: $1"; WARN=$((WARN+1)); }

# Parse arguments
USE_TARBALL=0
if [ "$1" == "--tarball" ]; then
	USE_TARBALL=1
	TARBALL="$2"
	if [ ! -f "$TARBALL" ]; then
		echo "ERROR: tarball not found: $TARBALL"
		exit 1
	fi
	echo "Verifying tarball: $TARBALL"
	echo "Extracting file list (this may take a moment)..."
	FILE_LIST=$(tar -tzf "$TARBALL" 2>/dev/null)
else
	SYSROOT="${1:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/staging/sysroot}"
	if [ ! -d "$SYSROOT" ]; then
		echo "ERROR: sysroot not found at $SYSROOT"
		exit 1
	fi
	echo "Verifying sysroot: $SYSROOT"
fi

check_lib() {
	local name="$1"
	if [ "$USE_TARBALL" -eq 1 ]; then
		echo "$FILE_LIST" | grep -qm1 "${name}\.so" && ok "$name (.so)" || fail "$name (.so)"
	else
		found=$(find "$SYSROOT" -name "${name}.so*" \( -type f -o -type l \) 2>/dev/null | head -1)
		[ -n "$found" ] && ok "$name (.so)" || fail "$name (.so)"
	fi
}

check_header() {
	local path="$1"
	local label="${2:-$path}"
	if [ "$USE_TARBALL" -eq 1 ]; then
		echo "$FILE_LIST" | grep -qm1 "usr/include.*${path}" && ok "$label (header)" || fail "$label (header)"
	else
		# Check both standard and multiarch include paths
		found=$(find "$SYSROOT/usr/include" -path "*/${path}" 2>/dev/null | head -1)
		[ -n "$found" ] && ok "$label (header)" || fail "$label (header)"
	fi
}

echo ""
echo "============================================"
echo "1. Core libraries (Qt6 build requirements)"
echo "============================================"
check_lib libglib-2.0;      check_header glib-2.0/glib.h glib
check_lib libssl;            check_header openssl/ssl.h openssl
check_lib libjpeg;           check_header jpeglib.h jpeg
check_lib libpng16;          check_header png.h png
check_lib libpcre2-16;       check_header pcre2.h pcre2
check_lib libharfbuzz;       check_header harfbuzz/hb.h harfbuzz
check_lib libfontconfig;     check_header fontconfig/fontconfig.h fontconfig
check_lib libfreetype;       check_header freetype2/freetype/freetype.h freetype
check_lib libdouble-conversion; check_header double-conversion/double-conversion.h double-conversion
check_lib libz;              check_header zlib.h zlib
check_lib libzstd;           check_header zstd.h zstd
check_lib libbz2;            check_header bzlib.h bzip2
check_lib liblzma;           check_header lzma.h lzma
check_lib libffi;            check_header ffi.h ffi
check_lib libmd;             check_header md5.h libmd
check_lib libbsd;            check_header bsd/bsd.h libbsd
check_lib libuuid;           check_header uuid/uuid.h uuid
check_lib libdbus-1;         check_header dbus-1.0/dbus/dbus.h dbus
check_lib libsqlite3;        check_header sqlite3.h sqlite3
check_lib libexpat;          check_header expat.h expat
check_lib libicuuc;          check_header unicode/uchar.h icu

echo ""
echo "============================================"
echo "2. Graphics / Display"
echo "============================================"
check_lib libEGL;            check_header EGL/egl.h EGL
check_lib libGLESv2;         check_header GLES2/gl2.h GLESv2
check_lib libdrm;            check_header libdrm/drm.h drm
check_lib libgbm;            check_header gbm.h gbm

echo ""
echo "============================================"
echo "3. XCB / X11"
echo "============================================"
check_lib libxcb;            check_header xcb/xcb.h xcb
check_lib libxcb-cursor;     check_header xcb/xcb_cursor.h xcb-cursor
check_lib libxcb-icccm;      check_header xcb/xcb_icccm.h xcb-icccm
check_lib libxcb-image;      check_header xcb/xcb_image.h xcb-image
check_lib libxcb-keysyms;    check_header xcb/xcb_keysyms.h xcb-keysyms
check_lib libxcb-randr;      check_header xcb/randr.h xcb-randr
check_lib libxcb-render;     check_header xcb/render.h xcb-render
check_lib libxcb-render-util; check_header xcb/xcb_renderutil.h xcb-render-util
check_lib libxcb-shape;      check_header xcb/shape.h xcb-shape
check_lib libxcb-shm;        check_header xcb/shm.h xcb-shm
check_lib libxcb-sync;       check_header xcb/sync.h xcb-sync
check_lib libxcb-xfixes;     check_header xcb/xfixes.h xcb-xfixes
check_lib libxcb-xkb;        check_header xcb/xkb.h xcb-xkb
check_lib libxkbcommon;      check_header xkbcommon/xkbcommon.h xkbcommon
check_lib libxkbcommon-x11;  check_header xkbcommon/xkbcommon-x11.h xkbcommon-x11
check_lib libX11-xcb;        check_header X11/Xlib-xcb.h X11-xcb

echo ""
echo "============================================"
echo "4. Wayland"
echo "============================================"
check_lib libwayland-client;  check_header wayland-client.h wayland-client
check_lib libwayland-server;  check_header wayland-server.h wayland-server

echo ""
echo "============================================"
echo "5. Input"
echo "============================================"
check_lib libinput;          check_header libinput.h libinput
check_lib libmtdev;          check_header mtdev.h mtdev
check_lib libudev;           check_header libudev.h udev

echo ""
echo "============================================"
echo "6. Boost"
echo "============================================"
check_lib libboost_system;           check_header boost/system/error_code.hpp boost-system
check_lib libboost_filesystem;       check_header boost/filesystem.hpp boost-filesystem
check_lib libboost_thread;           check_header boost/thread.hpp boost-thread
check_lib libboost_program_options;  check_header boost/program_options.hpp boost-program-options

echo ""
echo "============================================"
echo "7. Scopy dependencies"
echo "============================================"
check_lib libfftw3;          check_header fftw3.h fftw3
check_lib libgmp;            check_header gmp.h gmp
check_lib libxml2;           check_header libxml2/libxml/parser.h libxml2
check_lib libusb-1.0;        check_header libusb-1.0/libusb.h libusb
check_lib libsndfile;        check_header sndfile.h sndfile
check_lib libzmq;            check_header zmq.h zmq
check_lib libgsl;            check_header gsl/gsl_math.h gsl
check_lib libavahi-client;   check_header avahi-client/client.h avahi

echo ""
echo "============================================"
echo "8. Python (for libsigrokdecode)"
echo "============================================"
check_lib libpython3.13
if [ "$USE_TARBALL" -eq 1 ]; then
	echo "$FILE_LIST" | grep -qm1 "usr/include/python3.*/Python.h" && ok "Python.h (header)" || fail "Python.h (header)"
else
	found=$(find "$SYSROOT/usr/include" -name "Python.h" -path "*/python3*" 2>/dev/null | head -1)
	[ -n "$found" ] && ok "Python.h (header)" || fail "Python.h (header)"
fi
check_lib libreadline;       check_header readline/readline.h readline
check_lib libgdbm;           check_header gdbm.h gdbm
check_lib libdb-5.3;         check_header db.h db

echo ""
echo "============================================"
echo "9. Audio codecs (libsndfile transitive deps)"
echo "============================================"
check_lib libFLAC;           check_header FLAC/all.h FLAC
check_lib libvorbis;         check_header vorbis/codec.h vorbis
check_lib libogg;            check_header ogg/ogg.h ogg
check_lib libopus;           check_header opus/opus.h opus
check_lib libmpg123;         check_header mpg123.h mpg123
check_lib libmp3lame;        check_header lame/lame.h lame

echo ""
echo "============================================"
echo "10. Cross-compilation symlinks"
echo "============================================"
if [ "$USE_TARBALL" -eq 0 ]; then
	for link in asm gnu bits sys; do
		[ -e "$SYSROOT/usr/include/$link" ] && ok "/usr/include/$link symlink" || fail "/usr/include/$link symlink"
	done
	for obj in crt1.o crti.o crtn.o; do
		[ -e "$SYSROOT/usr/lib/$obj" ] && ok "/usr/lib/$obj symlink" || fail "/usr/lib/$obj symlink"
	done
else
	echo "  (skipped — symlinks not verifiable in tarball)"
fi

echo ""
echo "============================================"
echo "SUMMARY"
echo "============================================"
echo -e "  ${GREEN}Passed${NC}: $PASS"
echo -e "  ${RED}Failed${NC}: $FAIL"
echo -e "  ${YELLOW}Warnings${NC}: $WARN"
echo ""

if [ $FAIL -gt 0 ]; then
	echo -e "${RED}SYSROOT INCOMPLETE — $FAIL missing items${NC}"
	exit 1
else
	echo -e "${GREEN}SYSROOT OK${NC}"
	exit 0
fi

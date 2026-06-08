#!/bin/bash
# Verify Qt6 host + armhf cross tarballs are complete and correct
# Usage: ./verify_qt6_cross.sh
#        ./verify_qt6_cross.sh --tarball-armhf path/to/qt6-armhf-cross-installed.tar.gz \
#                              --tarball-host  path/to/qt6-host-installed.tar.gz

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0
WARN=0

ok()   { echo -e "  ${GREEN}OK${NC}: $1";      PASS=$((PASS+1)); }
fail() { echo -e "  ${RED}MISSING${NC}: $1";   FAIL=$((FAIL+1)); }
warn() { echo -e "  ${YELLOW}WARN${NC}: $1";   WARN=$((WARN+1)); }

# Defaults — installed paths (inside Docker or after manual extraction)
ARMHF_PREFIX=/opt/Qt/6.8.3/armhf
HOST_PREFIX=/opt/Qt/6.8.3/gcc_64

ARMHF_TARBALL=""
HOST_TARBALL=""
MODE=installed   # installed | tarball

while [ "$1" != "" ]; do
    case "$1" in
        --tarball-armhf) ARMHF_TARBALL="$2"; MODE=tarball; shift ;;
        --tarball-host)  HOST_TARBALL="$2";  MODE=tarball; shift ;;
    esac
    shift
done

if [ "$MODE" = "tarball" ]; then
    if [ -z "$ARMHF_TARBALL" ] || [ -z "$HOST_TARBALL" ]; then
        echo "ERROR: --tarball mode requires both --tarball-armhf and --tarball-host"
        exit 1
    fi
    [ -f "$ARMHF_TARBALL" ] || { echo "ERROR: not found: $ARMHF_TARBALL"; exit 1; }
    [ -f "$HOST_TARBALL"  ] || { echo "ERROR: not found: $HOST_TARBALL";  exit 1; }
    echo "Verifying tarballs (extracting file lists — may take a moment)..."
    ARMHF_LIST=$(tar -tzf "$ARMHF_TARBALL")
    HOST_LIST=$(tar -tzf "$HOST_TARBALL")
else
    echo "Verifying installed Qt6 at:"
    echo "  armhf: $ARMHF_PREFIX"
    echo "  host:  $HOST_PREFIX"
    [ -d "$ARMHF_PREFIX" ] || { echo "ERROR: armhf prefix not found: $ARMHF_PREFIX"; exit 1; }
    [ -d "$HOST_PREFIX"  ] || { echo "ERROR: host prefix not found: $HOST_PREFIX";   exit 1; }
fi

# ----------------------------------------------------------------
# Helpers
# ----------------------------------------------------------------

check_armhf_lib() {
    local name="$1"   # e.g. libQt6Core
    if [ "$MODE" = "tarball" ]; then
        echo "$ARMHF_LIST" | grep -qm1 "${name}\.so" && ok "$name" || fail "$name"
    else
        found=$(find "$ARMHF_PREFIX/lib" -name "${name}.so*" 2>/dev/null | head -1)
        [ -n "$found" ] && ok "$name" || fail "$name"
    fi
}

check_host_bin() {
    local name="$1"   # e.g. moc
    if [ "$MODE" = "tarball" ]; then
        echo "$HOST_LIST" | grep -qm1 "\(bin\|libexec\)/${name}$" && ok "host:$name" || fail "host:$name"
    else
        { [ -x "$HOST_PREFIX/bin/$name" ] || [ -x "$HOST_PREFIX/libexec/$name" ]; } && ok "host:$name" || fail "host:$name"
    fi
}

check_cmake_module() {
    local name="$1"   # e.g. Qt6Core
    if [ "$MODE" = "tarball" ]; then
        echo "$ARMHF_LIST" | grep -qm1 "cmake/${name}/${name}Config.cmake" && ok "cmake:$name" || fail "cmake:$name"
    else
        [ -f "$ARMHF_PREFIX/lib/cmake/${name}/${name}Config.cmake" ] && ok "cmake:$name" || fail "cmake:$name"
    fi
}

check_armhf_elf() {
    # Verify a .so is actually armhf ELF, not x86_64
    local name="$1"
    if [ "$MODE" = "tarball" ]; then
        warn "ELF arch check skipped in tarball mode (need extraction)"
        return
    fi
    local path
    path=$(find "$ARMHF_PREFIX/lib" -name "${name}.so*" -not -type l 2>/dev/null | head -1)
    if [ -z "$path" ]; then
        fail "ELF check: $name not found"
        return
    fi
    if file "$path" | grep -q "ARM"; then
        ok "ELF arch: $name is ARM"
    else
        fail "ELF arch: $name is NOT ARM ($(file "$path" | grep -o 'ELF.*'))"
    fi
}

check_target_qt_conf() {
    if [ "$MODE" = "tarball" ]; then
        echo "$ARMHF_LIST" | grep -qm1 "bin/target_qt.conf" && ok "target_qt.conf present" || fail "target_qt.conf present"
        if echo "$ARMHF_LIST" | grep -qm1 "bin/target_qt.conf"; then
            # Can't read contents from tarball listing alone — just note it
            warn "target_qt.conf content not verified in tarball mode"
        fi
    else
        local conf="$ARMHF_PREFIX/bin/target_qt.conf"
        [ -f "$conf" ] && ok "target_qt.conf present" || fail "target_qt.conf present"
        if [ -f "$conf" ]; then
            grep -q "Sysroot=" "$conf"                && ok "target_qt.conf: Sysroot set"      || fail "target_qt.conf: Sysroot set"
            grep -q "TargetSpec=linux-arm-gnueabihf"  "$conf" && ok "target_qt.conf: TargetSpec=armhf" || fail "target_qt.conf: TargetSpec=armhf"
        fi
    fi
}

check_mkspec() {
    if [ "$MODE" = "tarball" ]; then
        echo "$ARMHF_LIST" | grep -qm1 "mkspecs/linux-arm-gnueabihf-g++/qmake.conf" \
            && ok "mkspec linux-arm-gnueabihf-g++" || fail "mkspec linux-arm-gnueabihf-g++"
    else
        [ -f "$ARMHF_PREFIX/mkspecs/linux-arm-gnueabihf-g++/qmake.conf" ] \
            && ok "mkspec linux-arm-gnueabihf-g++" || fail "mkspec linux-arm-gnueabihf-g++"
    fi
}

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "1. Host tools (x86_64 — moc, rcc, uic)"
echo "============================================"
check_host_bin moc
check_host_bin rcc
check_host_bin uic
check_host_bin qmake6
check_host_bin qt-cmake

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "2. Core Qt6 armhf libraries (required by Scopy)"
echo "============================================"
check_armhf_lib libQt6Core
check_armhf_lib libQt6Gui
check_armhf_lib libQt6Widgets
check_armhf_lib libQt6Network
check_armhf_lib libQt6OpenGL
check_armhf_lib libQt6DBus
check_armhf_lib libQt6Xml
check_armhf_lib libQt6Svg
check_armhf_lib libQt6SvgWidgets
check_armhf_lib libQt6PrintSupport
check_armhf_lib libQt6Quick
check_armhf_lib libQt6Qml
check_armhf_lib libQt6QmlModels
check_armhf_lib libQt6Core5Compat

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "3. Platform plugins"
echo "============================================"
if [ "$MODE" = "tarball" ]; then
    echo "$ARMHF_LIST" | grep -qm1 "plugins/platforms/libqxcb.so"   && ok "platform: xcb"   || fail "platform: xcb"
    echo "$ARMHF_LIST" | grep -qm1 "plugins/platforms/libqeglfs.so" && ok "platform: eglfs" || fail "platform: eglfs"
    echo "$ARMHF_LIST" | grep -qm1 "plugins/platforms/libqminimal.so" && ok "platform: minimal" || fail "platform: minimal"
else
    [ -f "$ARMHF_PREFIX/plugins/platforms/libqxcb.so" ]   && ok "platform: xcb"   || fail "platform: xcb"
    [ -f "$ARMHF_PREFIX/plugins/platforms/libqeglfs.so" ] && ok "platform: eglfs" || fail "platform: eglfs"
    [ -f "$ARMHF_PREFIX/plugins/platforms/libqminimal.so" ] && ok "platform: minimal" || fail "platform: minimal"
fi

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "4. CMake modules (needed by Scopy cmake)"
echo "============================================"
check_cmake_module Qt6Core
check_cmake_module Qt6Gui
check_cmake_module Qt6Widgets
check_cmake_module Qt6Network
check_cmake_module Qt6OpenGL
check_cmake_module Qt6DBus
check_cmake_module Qt6Xml
check_cmake_module Qt6Svg
check_cmake_module Qt6PrintSupport
check_cmake_module Qt6Quick
check_cmake_module Qt6Qml
check_cmake_module Qt6Core5Compat
check_cmake_module Qt6UiTools

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "5. Cross-compilation config"
echo "============================================"
check_target_qt_conf
check_mkspec

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "6. ELF architecture check (armhf, not x86_64)"
echo "============================================"
check_armhf_elf libQt6Core
check_armhf_elf libQt6Gui
check_armhf_elf libQt6Widgets

# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "SUMMARY"
echo "============================================"
echo -e "  ${GREEN}Passed${NC}: $PASS"
echo -e "  ${RED}Failed${NC}: $FAIL"
echo -e "  ${YELLOW}Warnings${NC}: $WARN"
echo ""

if [ $FAIL -gt 0 ]; then
    echo -e "${RED}Qt6 cross install INCOMPLETE — $FAIL missing items${NC}"
    exit 1
else
    echo -e "${GREEN}Qt6 cross install OK${NC}"
    exit 0
fi

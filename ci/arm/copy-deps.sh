#!/bin/bash

# Library Dependency Scanner and Copier
# ====================================
# Recursively finds and copies all shared library dependencies
# Usage: ./copy-deps.sh [options] binary_files...
#
# Options:
#   --lib-dir PATH    Colon-separated paths to search for libraries
#   --output-dir DIR  Directory to copy libraries to
#   --help            Show usage information
#
# Examples:
#   # List dependencies only (no copy):
#   ./copy-deps.sh --lib-dir /usr/lib:/usr/local/lib /usr/bin/scopy
#
#   # Copy all dependencies:
#   ./copy-deps.sh --lib-dir /sysroot:/build --output-dir ./libs /usr/bin/scopy
#
# Uses the AppImage excludelist to skip system libraries that should not be bundled

set -e

usage()
{
	echo "Usage: $0 [--lib-dir PATH] [--output-dir DIR] binary_files..."
	echo "  --lib-dir PATH     Colon-separated library search paths"
	echo "  --output-dir DIR   Copy libraries to this directory"
	echo "  --help            Show this help message"
	echo ""
	echo "Without --output-dir, only lists dependencies without copying"
}

COPY=""

while [ "$1" != "" ]
do
    case "$1" in
        "--help"| "-h")
            usage
            exit
            ;;
        "--lib-dir" | "-l")
            # Add to library search path (can be used multiple times)
            SYSROOT="$2:$SYSROOT"
            shift
            ;;
        "--output-dir" | "-o")
            # Enable copy mode and set destination
            OUTPUT_DIR="$2"
            COPY="true"
            shift
            ;;
        *)
            # Assume it's a file to analyze
            FILES="$FILES $1"
            ;;
    esac
    shift
done

LIBS_ARRAY=()

# Convert colon-separated paths to array
IFS=":" read -r -a SEARCH_PATHS <<< "$SYSROOT"

# library blacklist
if [[ $COPY != "true" ]]; then
    # In list mode, show all dependencies
    BLACKLISTED=""
else
    # Fetch the excludelist explicitly instead of inside the array assignment. This script runs
    # under `set -e`, and in the old one-liner a network failure or an empty response made the
    # trailing `grep` exit 1; the assignment inherited that status and packaging aborted here with
    # no message at all - the AppDir was simply left without its libraries. Report the failure and
    # fall back to a built-in list rather than dying quietly.
    EXCLUDELIST_URL=https://raw.githubusercontent.com/probonopd/AppImages/master/excludelist
    EXCLUDELIST_RAW=""
    if ! EXCLUDELIST_RAW=$(wget --quiet --timeout=30 --tries=3 "$EXCLUDELIST_URL" -O -); then
        echo "WARNING: could not fetch the AppImage excludelist from $EXCLUDELIST_URL"
        EXCLUDELIST_RAW=""
    fi

    BLACKLISTED=($(echo "$EXCLUDELIST_RAW" | sort | uniq | cut -d '#' -f 1 | grep "[^-[:space:]]" || true))

    if [ ${#BLACKLISTED[@]} -eq 0 ]; then
        echo "WARNING: the AppImage excludelist came back empty - using the built-in fallback."
        echo "         This is a snapshot of the upstream list's library entries; it may be stale,"
        echo "         but bundling a library that should have come from the host is recoverable,"
        echo "         whereas aborting the library copy without a message is not."
        BLACKLISTED=(
            ld-linux.so.2 ld-linux-x86-64.so.2 libanl.so.1 libBrokenLocale.so.1
            libcidn.so.1 libc.so.6 libdl.so.2 libm.so.6
            libmvec.so.1 libnss_compat.so.2 libnss_dns.so.2 libnss_files.so.2
            libnss_hesiod.so.2 libnss_nisplus.so.2 libnss_nis.so.2 libpthread.so.0
            libresolv.so.2 librt.so.1 libthread_db.so.1 libutil.so.1
            libstdc++.so.6 libGL.so.1 libEGL.so.1 libGLdispatch.so.0
            libGLX.so.0 libOpenGL.so.0 libdrm.so.2 libglapi.so.0
            libgbm.so.1 libxcb.so.1 libX11.so.6 libX11-xcb.so.1
            libwayland-client.so.0 libasound.so.2 libfontconfig.so.1 libfreetype.so.6
            libharfbuzz.so.0 libcom_err.so.2 libexpat.so.1 libgcc_s.so.1
            libgpg-error.so.0 libICE.so.6 libSM.so.6 libusb-1.0.so.0
            libuuid.so.1 libz.so.1 libjack.so.0 libpipewire-0.3.so.0
            libxcb-dri3.so.0 libxcb-dri2.so.0 libfribidi.so.0 libgmp.so.10
        )
    fi
    BLACKLISTED+=(ld-linux-armhf.so)
    BLACKLISTED+=(ld-linux-aarch64.so)
    BLACKLISTED+=(ld-linux-armhf.so.3)
    BLACKLISTED+=(ld-linux-aarch64.so.1)
    BLACKLISTED+=(libglib-2.0.so.0)
fi


# Searches for a library by name in all configured search paths
findlib()
{
    LIB=$1
    for path in "${SEARCH_PATHS[@]}"
    do
        # Strip version suffix and search for any version
        # libfoo.so.1.2.3 -> search for libfoo.so*
        LIB_PATH=$(find "$path" -name "${LIB%%.so*}.so*" -type f  2>/dev/null | sort | head -1)
        [ "$LIB_PATH" != "" ] && break # Exit on first successful find
    done

    [ "$LIB_PATH" = "" ] && LIB_PATH="not found"
    echo "$LIB_PATH"
}

# Copies a library file and recreates all associated symbolic links
# This preserves the standard library naming convention:
#   libfoo.so -> libfoo.so.1 -> libfoo.so.1.2.3
copylib()
{
    library=$1
    destination=$2

    lib_location=${library%/*}     # Directory containing library
    lib_name=${library##*/}        # Library filename
    lib_name_without_version=${lib_name%%.so*}.so  # Base name

    echo "${lib_name_without_version}"
    echo -n copied:
    cp --verbose "${library}" "${destination}"

    # Find all symbolic links to this library
    symlinks=($(find "$lib_location" -name "${lib_name_without_version}*" -type l 2>/dev/null))
    for link in "${symlinks[@]}"
    do
       pushd ${destination} > /dev/null
       # Remove existing link if present
       [ -L "${link##*/}" ] && rm "${link##*/}"
       # Create new symbolic link
       echo -n linked:
       ln --symbolic --verbose "${library##*/}" "${link##*/}"
       popd > /dev/null
    done
}


# Use readelf to find NEEDED entries
# and recursively processes each dependency
process_lib(){
    DEPS=$(readelf -d "$1"  2> /dev/null | \
                grep NEEDED | \
                cut -d ":" -f 2 | \
                sed -e 's,\[,,g' -e 's,],,g' -e 's,[ ]*,,g'| \
                sort)

    for library in $DEPS
    do
        if  [[ ! "${BLACKLISTED[*]}" =~ "${library}"  && ! "${LIBS_ARRAY[*]}" =~ "${library}" ]]; then

        # Mark as processed
        LIBS_ARRAY+=("${library}")

        LIB_PATH=$(findlib "$library")

        if [[ $COPY == "true" ]]; then
            # Copy mode
            if [[ $LIB_PATH == "not found" ]]; then
                echo "Error: ${library} not found"
                exit 1
            fi
            mkdir -p "$OUTPUT_DIR"
            copylib "$LIB_PATH" "$OUTPUT_DIR"
        fi
        # List mode
        [ "$COPY" != "true" ] && echo "$library => $LIB_PATH"
        process_lib "$LIB_PATH"
        fi
    done
}

for arg in $FILES; do
    process_lib "${arg}"
done
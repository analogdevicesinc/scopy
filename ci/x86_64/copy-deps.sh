#!/bin/bash

# x86_64 Library Dependency Copier
# ===============================
# Purpose: Recursively copy shared library dependencies for AppImage
# Usage: ./copy-deps.sh binary_file destination_directory
#
# This script:
# 1. Uses ldd to find dependencies
# 2. Recursively processes each dependency
# 3. Excludes system libraries using AppImage excludelist
# 4. Strips debug symbols to reduce size

set -e
SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

# Script parameters
BINARY=$1           # Binary or library to analyze
LOCATION=$2         # Where to copy dependencies
LIBS_ARRAY=()       # Track processed libraries to avoid duplicates

# Download and parse AppImage excludelist
# These are system libraries that should not be bundled
BLACKLISTED=($(wget --quiet https://raw.githubusercontent.com/probonopd/AppImages/master/excludelist -O - | sort | uniq | cut -d '#' -f 1 | grep -v "^#.*" | grep "[^-\s]"))
BLACKLISTED+=(libglib-2.0.so.0)

# Add AppDir libraries to search path
# This ensures we find our bundled libraries first
export LD_LIBRARY_PATH="${APP_DIR}/usr/lib:${SRC_DIR}/build:$LD_LIBRARY_PATH"

# Recursive function to process library dependencies
# =================================================
# Uses ldd to find dependencies and recursively processes each one
run_ldd(){
	# Parse ldd output to extract library paths
	# ldd format: "libfoo.so.1 => /usr/lib/libfoo.so.1 (0x...)"
	for library in $(ldd "$1" | cut -d '>' -f 2 | awk '{print $1}')
	do
		# Skip if library is blacklisted
		if ! [[ "${BLACKLISTED[*]}" =~ "${library##*/}" ]]; then
			# Process if:
			# - File exists
			# - Not already processed (avoid infinite loops)
			if [ -f "${library}" ] && ! [[ "${LIBS_ARRAY[*]}" =~ "${library}" ]]; then
				# Mark as processed
				LIBS_ARRAY+=("${library}")
				echo "---Added new lib: ${library}"

				# Copy library if not already present
				if [ ! -f "${LOCATION}"/"${library##*/}" ]; then
					cp "${library}" "${LOCATION}"
					# If it's a symlink, also copy the target
					[ -L "${library}" ] && cp "$(realpath "${library}")" "${LOCATION}"
					# Strip debug symbols to reduce size
					strip --strip-unneeded "${LOCATION}"/"${library##*/}"
				fi

				# Recursively process this library's dependencies
				run_ldd "${library}"
			fi
		fi
	done
}

# Main execution
# Process each binary/library passed as argument
for arg in $BINARY; do
	run_ldd "${arg}"
done

#!/bin/bash

set -e
SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

BINARY=$1
LOCATION=$2
LIBS_ARRAY=()
BLACKLISTED=($(wget --quiet https://raw.githubusercontent.com/probonopd/AppImages/master/excludelist -O - | sort | uniq | cut -d '#' -f 1 | grep -v "^#.*" | grep "[^-\s]"))

export LD_LIBRARY_PATH="${APP_DIR}/usr/lib:${SRC_DIR}/build:$LD_LIBRARY_PATH"
run_ldd(){
	for library in $(ldd "$1" | cut -d '>' -f 2 | awk '{print $1}')
	do
		# check if the library exists at that path and if it was processed already or blacklisted
		if ! [[ "${BLACKLISTED[*]}" =~ "${library##*/}" ]]; then
			if [ -f "${library}" ] && ! [[ "${LIBS_ARRAY[*]}" =~ "${library}" ]]; then
				LIBS_ARRAY+=("${library}")
				echo "---Added new lib: ${library}"
				if [ ! -f "${LOCATION}"/"${library##*/}" ]; then
					cp "${library}" "${LOCATION}"
					[ -L "${library}" ] && cp "$(realpath "${library}")" "${LOCATION}"
					strip --strip-unneeded "${LOCATION}"/"${library##*/}"
				fi
				run_ldd "${library}"
			fi
		fi
	done
}

for arg in $BINARY; do
	run_ldd "${arg}"
done

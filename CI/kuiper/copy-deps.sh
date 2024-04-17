#!/bin/bash

set -e
SRC_DIR=$(git rev-parse --show-toplevel)
source $SRC_DIR/CI/kuiper/kuiper_build_config.sh

BINARY=$1
LOCATION=$2


if [ ! -f "${SRC_DIR}"/CI/kuiper/ldd-mod ]; then
	sed 's|.*RTLDLIST=.*|RTLDLIST="/usr/arm-linux-gnueabihf/lib/ld-2.31.so /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3"|' /usr/bin/ldd | tee "${SRC_DIR}"/CI/kuiper/ldd-mod
	chmod +x "${SRC_DIR}"/CI/kuiper/ldd-mod
fi

export LD_LIBRARY_PATH="${APP_DIR}/usr/lib:${SYSROOT}/lib:${SYSROOT}/lib/arm-linux-gnueabihf:${SYSROOT}/usr/arm-linux-gnueabihf/lib:${SYSROOT}/usr/local/qt5.15/lib:${SYSROOT}/usr/local/lib:${SRC_DIR}/build"
LIBS_ARRAY=()
run_ldd(){

	if [ ! -z "$(${SRC_DIR}/CI/kuiper/ldd-mod $1 | grep "not found")" ]; then
		echo "--- LIB NOT FOUND"
		${SRC_DIR}/CI/kuiper/ldd-mod $1
		exit 1
	fi

	for library in $("${SRC_DIR}"/CI/kuiper/ldd-mod "$1" | cut -d '>' -f 2 | awk '{print $1}')
	do
		# check if the library exists at that path and if it was processed already
		if [ -f "${library}" ] && ! [[ "${LIBS_ARRAY[*]}" =~ "${library}" ]]; then
			LIBS_ARRAY+=("${library}")
			echo "---NEW: ${library}"
			if [ ! -f "${LOCATION}"/"${library##*/}" ]; then
				cp --verbose "${library}" "${LOCATION}"
				[ -L "${library}" ] && cp --verbose "$(realpath "${library}")" "${LOCATION}"
			fi
			run_ldd "${library}"
		fi
	done
}

for arg in $BINARY; do
	run_ldd "${arg}"
done

#!/bin/bash
set -e

. CI/travis/lib.sh

handle_default() {
echo "HANDLE DEFAULT"
	# if we have a Qt59 installation use it
if [ -f /opt/qt59/bin/qt59-env.sh ] ; then
	set +e
	. /opt/qt59/bin/qt59-env.sh
	set -e
fi

NUM_JOBS=4

mkdir -p build
pushd build
if [ "$TRAVIS" == "true" ] ; then
	cmake ..
	make -j${NUM_JOBS}
else
	cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR;${QT_PATH}/lib/cmake" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" ..
	CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS}
fi

popd
}

handle_centos() {
	export PATH=/usr/lib64:/usr/local/lib/pkgconfig:$PATH
	export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

	handle_default
}

handle_centos_docker() {
		sudo docker run --rm=true \
			-v `pwd`:/scopy:rw \
			centos:${OS_VERSION} \
			/bin/bash -xe /scopy/CI/travis/inside_centos_docker.sh scopy
}

handle_ubuntu_docker() {
	sudo docker run --rm=true \
			-v `pwd`:/scopy:rw \
			ubuntu:${OS_VERSION} \
			/bin/bash -xe /scopy/CI/travis/inside_ubuntu_docker.sh scopy
}

handle_ubuntu_flatpak_docker() {
	sudo docker run --privileged --rm=true \
			-v `pwd`:/scopy:rw \
			alexandratr/ubuntu-flatpak-kde:latest \
			/bin/bash -xe /scopy/CI/travis/inside_ubuntu_flatpak_docker.sh
}

LIBNAME=${1:-home/travis/build/analogdevicesinc/scopy}
OS_TYPE=${2:-default}
OS_VERSION="$3"

handle_${OS_TYPE}

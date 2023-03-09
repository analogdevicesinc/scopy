#!/usr/bin/bash.exe

export MINGW_VERSION=mingw64
export ARCH=x86_64

export CC=/${MINGW_VERSION}/bin/${ARCH}-w64-mingw32-gcc.exe
export CXX=/${MINGW_VERSION}/bin/${ARCH}-w64-mingw32-g++.exe

export CMAKE_OPTS="
	-DCMAKE_C_COMPILER:FILEPATH=${CC} \
	-DCMAKE_CXX_COMPILER:FILEPATH=${CXX} \
	-DPKG_CONFIG_EXECUTABLE=/c/msys64/$MINGW_VERSION/bin/pkg-config.exe \
	-DCMAKE_PREFIX_PATH=/c/msys64/$MINGW_VERSION/lib/cmake \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	"

export SCOPY_CMAKE_OPTS="
	$RC_COMPILER_OPT \
	-DBREAKPAD_HANDLER=OFF \
	-DGIT_EXECUTABLE=/c/Program\\ Files/Git/cmd/git.exe \
	-DPYTHON_EXECUTABLE=/$MINGW_VERSION/bin/python3.exe \
	"

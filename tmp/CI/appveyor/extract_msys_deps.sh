#!/usr/bin/bash.exe

SCOPY_MINGW_BUILD_DEPS_FORK=analogdevicesinc
SCOPY_MINGW_BUILD_DEPS_BRANCH=master

echo "Download pre-downloaded MSYS2 libraries with required versions... "
wget "https://ci.appveyor.com/api/projects/$SCOPY_MINGW_BUILD_DEPS_FORK/scopy-mingw-build-deps/artifacts/old-msys-build-deps-$MINGW_VERSION.tar.xz?branch=$SCOPY_MINGW_BUILD_DEPS_BRANCH&job=Environment: MINGW_VERSION=$MINGW_VERSION, ARCH=$ARCH" -O /tmp/old-msys-build-deps-$MINGW_VERSION.tar.xz
cd /c
tar xvf /tmp/old-msys-build-deps-$MINGW_VERSION.tar.xz

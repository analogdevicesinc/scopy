#!/bin/bash
set -e

# Use the /usr/local/bin/pkg-config instead of /Mono/../pkg-config
export PATH="/usr/local/bin:$PATH"

# if we have a Qt59 installation use it
if [ -f /opt/qt59/bin/qt59-env.sh ] ; then
	. /opt/qt59/bin/qt59-env.sh
fi

if command -v brew ; then
	QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
	export PATH="${QT_PATH}:$PATH"
fi

NUM_JOBS=4

mkdir -p build

cd build

if [ "$APPVEYOR" == "true" ] ; then
	MACOS_VERSION=$(/usr/libexec/PlistBuddy -c "Print:ProductVersion" /System/Library/CoreServices/SystemVersion.plist)
	if [[ "$MACOS_VERSION" == "10.14."* ]] ; then
		export MACOSX_DEPLOYMENT_TARGET=10.13
	fi
	cmake ..
	make -j${NUM_JOBS}
	otool -l ./Scopy.app/Contents/MacOS/Scopy # for debugging
else
	cmake -DCMAKE_PREFIX_PATH="$STAGINGDIR;${QT_PATH}/lib/cmake" -DCMAKE_INSTALL_PREFIX="$STAGINGDIR" \
		-DCMAKE_EXE_LINKER_FLAGS="-L${STAGINGDIR}/lib" ..
	CFLAGS=-I${STAGINGDIR}/include LDFLAGS=-L${STAGINGDIR}/lib make -j${NUM_JOBS}
fi


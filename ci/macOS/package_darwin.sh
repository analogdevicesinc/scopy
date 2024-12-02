#!/bin/bash
set -ex
STAGING_AREA=$PWD/staging
STAGINGDIR=$STAGING_AREA/dependencies

cd build
mkdir -p ./Scopy.app/Contents/Frameworks

## Handle libm2k paths
m2kpath=${STAGINGDIR}/lib/libm2k.dylib
m2krpath="$(otool -D ${m2kpath} | grep @rpath)"
m2kid=${m2krpath#"@rpath/"}
sudo cp ${STAGINGDIR}/lib/libm2k.* ./Scopy.app/Contents/Frameworks
sudo install_name_tool -id @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/${m2kid}
sudo install_name_tool -change ${m2krpath} @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/MacOS/Scopy

export DYLD_FALLBACK_LIBRARY_PATH=${STAGINGDIR}/lib

# Copy the iio and ad9361 to the stagingdir path
sudo cp -R /Library/Frameworks/iio.framework ${STAGINGDIR}/lib
sudo cp -R /Library/Frameworks/ad9361.framework ${STAGINGDIR}/lib

## Bundle some known dependencies
# -ns == no signing
sudo echo "${STAGINGDIR}/lib" | dylibbundler -ns -od -b -x ./Scopy.app/Contents/MacOS/Scopy -d ./Scopy.app/Contents/Frameworks/ -p @executable_path/../Frameworks/ >/dev/null

## Copy the frameworks dylibbundler failed to copy
sudo cp -R /usr/local/opt/python/Frameworks/Python.framework Scopy.app/Contents/Frameworks/
sudo cp -R /Library/Frameworks/iio.framework Scopy.app/Contents/Frameworks/
sudo cp -R /Library/Frameworks/ad9361.framework Scopy.app/Contents/Frameworks/

## Handle those framework paths
iiorpath="$(otool -D ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep @rpath)"
ad9361rpath="$(otool -D ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361 | grep @rpath)"
if [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.8/Python ] ; then
	pyversion=3.8
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.8/Python | head -2 |  tail -1)"
elif [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.9/Python ] ; then
	pyversion=3.9
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.9/Python | head -2 |  tail -1)"
elif [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.10/Python ] ; then
	pyversion=3.10
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.10/Python | head -2 |  tail -1)"
else
	echo "No Python 3.10, 3.9, 3.8 paths found"
	exit 1
fi
libusbpath="$(otool -L ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep libusb | cut -d " " -f 1)"
libusbid="$(echo ${libusbpath} | rev | cut -d "/" -f 1 | rev)"

sudo cp ${libusbpath} ./Scopy.app/Contents/Frameworks/

iioid=${iiorpath#"@rpath/"}
ad9361id=${ad9361rpath#"@rpath/"}
if [ "${pyversion}" = "3.8" ] ; then
	pythonid=${pythonidrpath#"/usr/local/opt/python@3.8/Frameworks/"}
elif [ "${pyversion}" = "3.9" ] ; then
	pythonid=${pythonidrpath#"/usr/local/opt/python@3.9/Frameworks/"}
elif [ "${pyversion}" = "3.10" ] ; then
	pythonid=${pythonidrpath#"/usr/local/opt/python@3.10/Frameworks/"}
else
	pythonid=${pythonidrpath#"/usr/local/opt/python/Frameworks/"}
fi

## Continue to handle those framework paths
sudo install_name_tool -id @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/iio.framework/iio
sudo install_name_tool -id @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/${iioid}
sudo install_name_tool -id @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361
sudo install_name_tool -id @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/Frameworks/${ad9361id}
sudo install_name_tool -id @executable_path/../Frameworks/${pythonid} ./Scopy.app/Contents/Frameworks/${pythonid}
sudo install_name_tool -id @executable_path/../Frameworks/${libusbid} ./Scopy.app/Contents/Frameworks/${libusbid}

sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/MacOS/Scopy
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/${ad9361id}
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libm2k*
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libgnuradio-iio*
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libgnuradio-m2k*
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libgnuradio-scopy*
sudo install_name_tool -change ${m2krpath} @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/libgnuradio-m2k*
sudo install_name_tool -change ${m2krpath} @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/libgnuradio-scopy*
sudo install_name_tool -change ${ad9361rpath} @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/MacOS/Scopy
sudo install_name_tool -change ${ad9361rpath} @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/Frameworks/libgnuradio-iio*
sudo install_name_tool -change ${pythonidrpath} @executable_path/../Frameworks/${pythonid} ./Scopy.app/Contents/Frameworks/libsigrokdecode*
sudo install_name_tool -change ${libusbpath} @executable_path/../Frameworks/${libusbid} ./Scopy.app/Contents/Frameworks/iio.framework/iio

if command -v brew ; then
	QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
	export PATH="${QT_PATH}:$PATH"
fi

## Handle iio-emu + libtinyiiod
sudo cp ./iio-emu/iio-emu ./Scopy.app/Contents/MacOS/
tinypath=${STAGINGDIR}/lib/tinyiiod.dylib
tinyrpath="$(otool -D ${tinypath} | grep @rpath)"
tinyid=${tinyrpath#"@rpath/"}
sudo cp ${STAGINGDIR}/lib/tinyiiod.* ./Scopy.app/Contents/Frameworks
sudo install_name_tool -id @executable_path/../Frameworks/${tinyid} ./Scopy.app/Contents/Frameworks/${tinyid}
sudo install_name_tool -change ${tinyrpath} @executable_path/../Frameworks/${tinyid} ./Scopy.app/Contents/MacOS/iio-emu

## Bundle the Qt libraries
sudo macdeployqt Scopy.app

curl -o /tmp/macdeployqtfix.py https://raw.githubusercontent.com/aurelien-rainone/macdeployqtfix/master/macdeployqtfix.py
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/Scopy ${QT_PATH}
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/iio-emu ${QT_PATH}
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/Scopy ./Scopy.app/Contents/Frameworks/

sudo macdeployqt Scopy.app -dmg


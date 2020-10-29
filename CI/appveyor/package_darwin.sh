#!/bin/bash
set -e
cd build

## Handle libm2k paths
mkdir -p ./Scopy.app/Contents/Frameworks
m2kpath=/usr/local/lib/libm2k.dylib
m2krpath="$(otool -D ${m2kpath} | grep @rpath)"
m2kid=${m2krpath#"@rpath/"}
sudo cp /usr/local/lib/libm2k.* ./Scopy.app/Contents/Frameworks
sudo install_name_tool -id @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/${m2kid}
sudo install_name_tool -change ${m2krpath} @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/MacOS/Scopy

## Handle qwtpolar paths
qwtpolarpath=/usr/local/lib/libqwtpolar.1.dylib
sudo install_name_tool -id ${qwtpolarpath} /usr/local/lib/libqwtpolar.dylib
sudo install_name_tool -change libqwtpolar.1.dylib ${qwtpolarpath} ./Scopy.app/Contents/MacOS/Scopy

export DYLD_FALLBACK_LIBRARY_PATH=/usr/local/lib
## Bundle some known dependencies
sudo echo "/usr/local/lib" | dylibbundler -od -b -x ./Scopy.app/Contents/MacOS/Scopy -d ./Scopy.app/Contents/Frameworks/ -p @executable_path/../Frameworks/ >/dev/null

## Copy the frameworks dylibbundler failed to copy
sudo cp -R /usr/local/opt/python/Frameworks/Python.framework Scopy.app/Contents/Frameworks/
sudo cp -R /Library/Frameworks/iio.framework Scopy.app/Contents/Frameworks/
sudo cp -R /Library/Frameworks/ad9361.framework Scopy.app/Contents/Frameworks/

## Handle those framework paths
iiorpath="$(otool -D ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep @rpath)"
ad9361rpath="$(otool -D ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361 | grep @rpath)"
if [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/Python ] ; then
	pyversion=3.7
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/Python | head -2 |  tail -1)"
elif [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.6/Python ] ; then
	pyversion=3.6
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.6/Python | head -2 |  tail -1)"
elif [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.8/Python ] ; then
	pyversion=3.8
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.8/Python | head -2 |  tail -1)"
elif [ -e /usr/local/opt/python/Frameworks/Python.framework/Versions/3.9/Python ] ; then
	pyversion=3.9
	pythonidrpath="$(otool -D /usr/local/opt/python/Frameworks/Python.framework/Versions/3.9/Python | head -2 |  tail -1)"
else
	echo "No Python 3.9, 3.8, 3.7 or 3.6 paths found"
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

## Bundle the Qt libraries
sudo /usr/local/opt/qt/bin/macdeployqt Scopy.app

curl -o /tmp/macdeployqtfix.py https://raw.githubusercontent.com/aurelien-rainone/macdeployqtfix/master/macdeployqtfix.py
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/Scopy /usr/local/opt/qt/
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/Scopy ./Scopy.app/Contents/Frameworks/

sudo /usr/local/opt/qt/bin/macdeployqt Scopy.app -dmg
ls

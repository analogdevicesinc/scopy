#!/bin/bash
set -ex
REPO_SRC=$(git rev-parse --show-toplevel)
source $REPO_SRC/ci/macOS/macos_config.sh

pushd $BUILDDIR

SCOPYPLUGINS=$(find $BUILDDIR/Scopy.app/Contents/MacOs/plugins -name "*.dylib" -type f)
SCOPYLIBS=$(find $BUILDDIR/Scopy.app/Contents/Frameworks -name "*.dylib" -type f)

echo "### Copy DLLs to Frameworks folder"
cp -R $REPO_SRC/plugins/dac/res/csv Scopy.app/Contents/MacOS/plugins/
cp -R $STAGING_AREA/libiio/build/iio.framework Scopy.app/Contents/Frameworks/
cp -R $STAGING_AREA/libad9361/build/ad9361.framework Scopy.app/Contents/Frameworks/
cp -R $BUILDDIR/plugins/emu_xml Scopy.app/Contents/MacOS/plugins
mkdir -p Scopy.app/Contents/MacOS/plugins/resources
cp $REPO_SRC/resources/scopy_emu_options_config.json Scopy.app/Contents/MacOS/plugins/resources/
cp -R $BUILDDIR/translations Scopy.app/Contents/MacOS
cp -R $BUILDDIR/plugins/regmap/xmls Scopy.app/Contents/MacOS/plugins

cp -r $BUILDDIR/translations $BUILDDIR/Scopy.app/Contents/MacOS/translations
cp -r $BUILDDIR/style $BUILDDIR/Scopy.app/Contents/MacOS/style

libqwtpath=${STAGING_AREA_DEPS}/lib/libqwt.6.4.0.dylib #hardcoded
libqwtid="$(otool -D ${libqwtpath} | tail -1)"
echo "=== Fixing libqwt"
[ -z "$(otool -L ${libqwtpath} | grep libqwt...dylib)" ] || install_name_tool -id ${libqwtid} ${libqwtpath}
otool -L ${libqwtpath}
install_name_tool -change ${libqwtid} ${libqwtpath} ./Scopy.app/Contents/MacOS/Scopy
for dylib in ${SCOPYLIBS} ${SCOPYPLUGINS}
do
	[ -z "$(otool -L ${dylib} | grep libqwt...dylib)" ] || install_name_tool -change ${libqwtid} ${libqwtpath} ${dylib}
	otool -L $dylib
done


iiorpath="$(otool -D ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep @rpath)"
iioid=${iiorpath#"@rpath/"}

ad9361rpath="$(otool -D ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361 | grep @rpath)"
ad9361id=${ad9361rpath#"@rpath/"}

libusbpath="$(otool -L ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep libusb | cut -d " " -f 1 | awk '{$1=$1};1')"
libusbid="$(echo ${libusbpath} | rev | cut -d "/" -f 1 | rev)"
cp ${libusbpath} ./Scopy.app/Contents/Frameworks/

m2kpath=${STAGING_AREA_DEPS}/lib/libm2k.dylib
m2krpath="$(otool -D ${m2kpath} | grep @rpath)"
m2kid=${m2krpath#"@rpath/"}
cp ${STAGING_AREA_DEPS}/lib/libm2k.* ./Scopy.app/Contents/Frameworks
install_name_tool -id @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/${m2kid}


echo "### Check available python version"
for version in 3.8 3.9 3.10 3.11 3.12
do
	if [ -e $(brew --prefix python3)/Frameworks/Python.framework/Versions/$version/Python ] ; then
		pythonpath=$(brew --prefix python3)/Frameworks/Python.framework/Versions/$version/Python
		pyversion=$version
		pythonidrpath="$(otool -D $pythonpath | head -2 |  tail -1)"
	fi
done

if [ -z $pyversion ] ; then
	echo "No Python 3.8, 3.9, 3.10, 3.11, 3.12 paths found"
	exit 1
fi
echo " - Found python$version at $pythonpath"
pythonid=${pythonidrpath#"$(brew --prefix python3)/Frameworks/"}
cp -R $(brew --prefix python3)/Frameworks/Python.framework Scopy.app/Contents/Frameworks/

echo "### Fixing scopy libraries and plugins "
for dylib in ${SCOPYLIBS} ${SCOPYPLUGINS}
do
	echo "--- FIXING LIB: ${dylib##*/}"
	echo $STAGING_AREA_DEPS/lib | dylibbundler --no-codesign --overwrite-files --bundle-deps --create-dir \
		--fix-file $dylib \
		--dest-dir $BUILDDIR/Scopy.app/Contents/Frameworks/ \
		--install-path @executable_path/../Frameworks/ \
		--search-path $BUILDDIR/Scopy.app/Contents/Frameworks/
done


echo "### Fixing Scopy binary"
echo $STAGING_AREA_DEPS/lib | dylibbundler -ns -of -b \
	-x $BUILDDIR/Scopy.app/Contents/MacOS/Scopy \
	-d $BUILDDIR/Scopy.app/Contents/Frameworks  \
	-p @executable_path/../Frameworks \
	-s $BUILDDIR/Scopy.app/Contents/Frameworks

echo "### Fixing the frameworks dylibbundler failed to copy"
echo "=== Fixing iio.framework"
install_name_tool -id @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/iio.framework/iio
install_name_tool -id @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/${iioid}
install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/MacOS/Scopy
for dylib in ${SCOPYLIBS} ${SCOPYPLUGINS}
do
	otool -L $dylib
	[ -z "$(otool -L ${dylib}| grep iio.framework)" ] && echo "SKIP ${dylib##*/}" || install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ${dylib}
done


echo "=== Fixing ad9361.framework"
install_name_tool -id @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361
install_name_tool -id @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/Frameworks/${ad9361id}
install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/${ad9361id}
install_name_tool -change ${ad9361rpath} @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/Frameworks/libgnuradio-iio*

echo "=== Fixing libusb"
install_name_tool -id @executable_path/../Frameworks/${libusbid} ./Scopy.app/Contents/Frameworks/${libusbid}
install_name_tool -change ${libusbpath} @executable_path/../Frameworks/${libusbid} ./Scopy.app/Contents/Frameworks/iio.framework/iio

echo "=== Fixing python"
install_name_tool -id @executable_path/../Frameworks/${pythonid} ./Scopy.app/Contents/Frameworks/${pythonid}
python_sigrokdecode=$(otool -L ./Scopy.app/Contents/Frameworks/libsigrokdecode* | grep python | cut -d " " -f 1 | awk '{$1=$1};1')
install_name_tool -change ${python_sigrokdecode} @executable_path/../Frameworks/${pythonid} ./Scopy.app/Contents/Frameworks/libsigrokdecode*
python_scopy=$(otool -L ./Scopy.app/Contents/MacOS/Scopy | grep -i python | cut -d " " -f 1 | awk '{$1=$1};1')
install_name_tool -change ${python_scopy} @executable_path/../Frameworks/${pythonid} ./Scopy.app/Contents/MacOS/Scopy
for dylib in ${SCOPYLIBS} ${SCOPYPLUGINS}
do
	otool -L $dylib
	python=$(otool -L ${dylib} | grep -i python | cut -d " " -f 1 | awk '{$1=$1};1');
	[ -z "${python}" ] && echo "SKIP ${dylib##*/}" || install_name_tool -change ${python} @executable_path/../Frameworks/${pythonid} ${dylib}
done


echo "=== Fixing libserialport"
libserialportpath="$(otool -L ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep libserialport | cut -d " " -f 1 | awk '{$1=$1};1')"
libserialportid="$(echo ${libserialportpath} | rev | cut -d "/" -f 1 | rev)"
install_name_tool -change ${libserialportpath} @executable_path/../Frameworks/${libserialportid} ./Scopy.app/Contents/Frameworks/iio.framework/iio

install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libm2k.dylib
install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libm2k.?.?.?.dylib
install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libgnuradio-m2k*
install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libgnuradio-scopy*
install_name_tool -change ${m2krpath} @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/libgnuradio-m2k*
install_name_tool -change ${m2krpath} @executable_path/../Frameworks/${m2kid} ./Scopy.app/Contents/Frameworks/libgnuradio-scopy*


echo "=== Fixing iio-emu + libtinyiiod"
cp $REPO_SRC/iio-emu/build/iio-emu ./Scopy.app/Contents/MacOS/
echo $STAGING_AREA_DEPS/lib | dylibbundler -ns -of -b \
	--fix-file $BUILDDIR/Scopy.app/Contents/MacOS/iio-emu \
	--dest-dir $BUILDDIR/Scopy.app/Contents/Frameworks/ \
	--install-path @executable_path/../Frameworks/ \
	--search-path $BUILDDIR/Scopy.app/Contents/Frameworks/

echo "=== Bundle the Qt libraries & Create Scopy.dmg"
macdeployqt Scopy.app -verbose=3
zip -Xvr ScopyApp.zip Scopy.app
macdeployqt Scopy.app -dmg -verbose=3
popd

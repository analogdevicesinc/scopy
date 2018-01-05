#!/bin/sh

sudo echo "/usr/local/lib" | dylibbundler -od -b -x ./Scopy.app/Contents/MacOS/Scopy -d ./Scopy.app/Contents/Frameworks/ -p @executable_path/../Frameworks/ >/dev/null

sudo cp -R /usr/local/opt/python3/Frameworks/Python.framework Scopy.app/Contents/Frameworks/
sudo cp -R /Library/Frameworks/iio.framework Scopy.app/Contents/Frameworks/
sudo cp -R /Library/Frameworks/ad9361.framework Scopy.app/Contents/Frameworks/

iiorpath="$(otool -D ./Scopy.app/Contents/Frameworks/iio.framework/iio | grep @rpath)"
ad9361rpath="$(otool -D ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361 | grep @rpath)"
pythonidrpath="$(otool -D /usr/local/opt/python3/Frameworks/Python.framework/Versions/3.6/Python | head -2 |  tail -1)"

iioid=${iiorpath#"@rpath/"}
ad9361id=${ad9361rpath#"@rpath/"}
pythonid=${pythonidrpath#"/usr/local/opt/python3/Frameworks/"}

sudo install_name_tool -id ${iioid} ./Scopy.app/Contents/Frameworks/iio.framework/iio
sudo install_name_tool -id ${iioid} ./Scopy.app/Contents/Frameworks/${iioid}
sudo install_name_tool -id ${ad9361id} ./Scopy.app/Contents/Frameworks/ad9361.framework/ad9361
sudo install_name_tool -id ${ad9361id} ./Scopy.app/Contents/Frameworks/${ad9361id}
sudo install_name_tool -id ${pythonid} ./Scopy.app/Contents/Frameworks/${pythonid}

sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/MacOS/Scopy
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/${ad9361id}
sudo install_name_tool -change ${iiorpath} @executable_path/../Frameworks/${iioid} ./Scopy.app/Contents/Frameworks/libgnuradio-iio*
sudo install_name_tool -change ${ad9361rpath} @executable_path/../Frameworks/${ad9361id} ./Scopy.app/Contents/MacOS/Scopy
sudo install_name_tool -change ${pythonidrpath} @executable_path/../Frameworks/${pythonid} ./Scopy.app/Contents/Frameworks/libsigrokdecode*

sudo /usr/local/opt/qt/bin/macdeployqt Scopy.app

curl -o /tmp/macdeployqtfix.py https://raw.githubusercontent.com/aurelien-rainone/macdeployqtfix/master/macdeployqtfix.py
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/Scopy /usr/local/opt/qt/
sudo python /tmp/macdeployqtfix.py ./Scopy.app/Contents/MacOS/Scopy ./Scopy.app/Contents/Frameworks/

sudo /usr/local/opt/qt/bin/macdeployqt Scopy.app -dmg
mkdir -p  ${TRAVIS_BUILD_DIR}/build/deploy
cp Scopy.dmg ${TRAVIS_BUILD_DIR}/build/deploy/scopy_latest${LDIST}.dmg
curl --upload-file ${TRAVIS_BUILD_DIR}/build/deploy/scopy_latest${LDIST}.dmg https://transfer.sh/scopy_latest${LDIST}.dmg

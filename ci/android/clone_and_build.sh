#!/bin/sh
wget https://raw.githubusercontent.com/analogdevicesinc/scopy/master/CI/appveyor/build_scopy_apk.sh
chmod +x ./build_scopy_apk.sh
./build_scopy_apk.sh

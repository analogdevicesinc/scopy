#! /bin/bash

# get scopy buil path from CMakeCache.txt
scopyBuildDir=$(grep 'SCOPY_BUILD_PATH' ../CMakeCache.txt  | awk -F= '{print $2}')
scopySourceDir=$(grep 'SCOPY_SOURCE_PATH' ../CMakeCache.txt  | awk -F= '{print $2}')

testScript=$(find $scopySourceDir -name "scopyDeviceConnectionTest.js");

if [ ! -z $testScript ]; then
    # start emu
    emuXmlPath="$scopySourceDir/tests/emuXml";
    cd $emuXmlPath
    iio-emu generic *.xml &

    # go to where scopy is build and run it with the scipt
    cd $scopyBuildDir
    QT_QPA_PLATFORM=offscreen ./scopy -s "$testScript"

    #stop emu
    killall -9 iio-emu
fi


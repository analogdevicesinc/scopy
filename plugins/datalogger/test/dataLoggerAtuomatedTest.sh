#! /bin/bash

# run all the tests in this file as CTest ?? in CMAKE

# get scopy buil path from CMakeCache.txt
scopyBuildDir=$(grep 'SCOPY_BUILD_PATH' ../CMakeCache.txt  | awk -F= '{print $2}')
scopySourceDir=$(grep 'SCOPY_SOURCE_PATH' ../CMakeCache.txt  | awk -F= '{print $2}')

isEmuRunning=TRUE

testScript=$(find $scopySourceDir -name "dataLoggerAutomatedTest.js");
if [ ! -z $testScript ]; then

    #check if emu is running
    if ! pgrep -x "iio-emu" > /dev/null
    then
	# if no emu running start emu for datalogger
	isEmuRunning=FALSE
	emuXmlPath=$scopySourceDir/plugins/datalogger/test/emuXml
	cd $emuXmlPath
	iio-emu generic *.xml &
    fi

    cd $scopyBuildDir
    # run scopy with the wanted script
    QT_QPA_PLATFORM=offscreen ./scopy -s "$testScript"

    # kill emu process if started by this test
    if [ "$isEmuRunning" = FALSE ]; then
	#stop emu
	killall -9 iio-emu
    fi
fi



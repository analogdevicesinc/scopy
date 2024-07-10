#! /bin/bash 
# set -xe
# get scopy buil path from CMakeCache.txt
scopyBuildDir=$(grep 'SCOPY_BUILD_PATH' ../CMakeCache.txt  | awk -F= '{print $2}')

isEmuRunning=TRUE

#if emu is not running start emu at provided path
if ! pgrep -x "iio-emu" > /dev/null; then
    isEmuRunning=FALSE
    if [ ! -z $1 ]; then
    echo "############# EMU PATH IS " $1;
        cd $1
        iio-emu generic *.xml &
    fi
fi

# run scopy functions of provided script
if [ ! -z $2 ]; then
    cd $scopyBuildDir
    ./scopy  --script=$2
fi

#if emu was started by this script stop emu
if [ "$isEmuRunning" = FALSE ]; then
	    #stop emu
	    killall -9 iio-emu
fi


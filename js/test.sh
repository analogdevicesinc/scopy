#! /bin/bash 
#
# Copyright (c) 2024 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

# set -xe
# get scopy build path from CMakeCache.txt
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
    QT_QPA_PLATFORM=offscreen ./scopy  --script=$2
fi

#if emu was started by this script stop emu
if [ "$isEmuRunning" = FALSE ]; then
    #stop emu
    killall -9 iio-emu
fi


#!/bin/bash
set -x

scopymodules=("core" "pluginbase" "gui" "iioutil" "iio-widgets" "gr-util")
scopyplugins=("test" "test2" "swiot" "adc" "pqm" "debugger" "datalogger" "regmap")

for mod in ${scopymodules[@]};do
	echo ${mod}
	./includemocs.py --header-prefix ../${mod}/include/${mod}/ --source-prefix ../${mod}/src
done

for mod in ${scopyplugins[@]};do
	echo ${mod}
	./includemocs.py --header-prefix ../plugins/${mod}/include/${mod}/ --source-prefix ../plugins/${mod}/src
done

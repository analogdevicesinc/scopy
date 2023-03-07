#!/bin/bash
set -x

scopymodules=("core" "pluginbase" "gui" "iioutil")
scopyplugins=("testplugin" "testplugin2" "swiot")

for mod in ${scopymodules[@]};do
	echo ${mod}
	./includemocs.py --header-prefix ../${mod}/include/${mod}/ --source-prefix ../${mod}/src
done

for mod in ${scopyplugins[@]};do
	echo ${mod}
	./includemocs.py --header-prefix ../plugins/${mod}/include/${mod}/ --source-prefix ../plugins/${mod}/src
done

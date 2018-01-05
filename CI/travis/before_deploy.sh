#!/bin/sh

deploy=0
if [ -z "$TRAVIS_BUILD_DIR" ] ; then
	t=$(find ./ -name CMakeCache.txt|head -1)
	if [ -n "${t}" ] ; then
		cd $(dirname $(dirname ${t}))
		TRAVIS_BUILD_DIR=$(pwd)
	else
		echo "I am confused - can't find CMakeCache.txt"
		exit
	fi
else
	cd $TRAVIS_BUILD_DIR
fi
pwd

if [ -z "${LDIST}" ] ; then
	LDIST=-$(lsb_release -c | awk '{print $NF}')
fi

check_file()
{
temp=""
for i in $(find ./ -name CMakeCache.txt)
do
hit=$(find $(dirname ${i}) -maxdepth 1 -name "scopy*.$1" | grep -v -- ${LDIST})
if [ "$(echo ${hit} | wc -w)" -gt "1"  ] ; then
	echo "I am confused - more than 2 $1 files!"
	echo $hit
	exit 1
else
	if [ "$(echo ${hit} | wc -w)" -eq "1" ] ; then
		if [ -z "${temp}" ] ; then
			temp=$hit
		else
			echo "I am confused - more than 2 $1 files"
			echo $temp
			echo $hit
			exit 1
		fi
	fi
fi
done
}

check_file dmg
if [ -n "${temp}" ] ; then
	deploy=$(expr ${deploy} + 1)
	if [ -z "${TARGET_DMG}" ] ; then
		export TARGET_DMG=$(echo ${temp} | \
			sed -e 's:^./.*/::' -e 's:.dmg$::')${LDIST}.dmg
	fi
	echo "deploying ${temp} to nightly $TARGET_DMG"
	if [ -z "${RELEASE_PKG_FILE_DMG}" ] ; then
		export RELEASE_PKG_FILE_DMG=$(dirname ${temp})/${TARGET_DMG}
		cp ${temp} ${RELEASE_PKG_FILE_DMG}
	fi
	echo ${TARGET_DMG}
	ls -lh ${temp}
	echo ${RELEASE_PKG_FILE_DMG}
	ls -lh ${RELEASE_PKG_FILE_DMG}
else
	echo "Skipping deployment of OS X package"
fi

if [ "${deploy}" -eq "0" ] ; then
	echo did not deploy any files
	exit 1
fi

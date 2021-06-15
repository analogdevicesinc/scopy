#!/bin/bash
set -xe
source $ANDROID_TOOLCHAIN_LOCATION/android_toolchain.sh $1 $2

if [ -n "$BRANCH" ]; then
	ARTIFACT_LOCATION=/home/runner/artifacts
else
	ARTIFACT_LOCATION=$GITHUB_WORKSPACE
fi

build_scopy() {
	pushd $WORKDIR
	rm -rf scopy

	git clone https://github.com/adisuciu/scopy.git --branch android3
	cd ${WORKDIR}/scopy
	rm -rf build*

	cp $SCRIPT_HOME_DIR/android_cmake.sh .
	cp $SCRIPT_HOME_DIR/android_deploy_qt.sh .

	./android_cmake.sh .

	cd build_$ABI
	make -j$JOBS
	make -j$JOBS install
	cd ..

	./android_deploy_qt.sh
	
	popd
}

move_artifact() {
	sudo cp $WORKDIR/scopy/build_$ABI/android-build/build/outputs/apk/debug/android-build-debug.apk $ARTIFACT_LOCATION/
}

build_scopy
move_artifact

#!/bin/bash
set -xe
source $ANDROID_TOOLCHAIN_LOCATION/android_toolchain.sh $1 $2

ARTIFACT_LOCATION=$GITHUB_WORKSPACE

build_scopy() {
	pushd $WORKDIR
	rm -rf scopy

	git clone https://github.com/analogdevicesinc/scopy.git

	cd ${WORKDIR}/scopy
	git fetch origin $BRANCH
	git checkout FETCH_HEAD

	rm -rf build*

	cp $BUILD_ROOT/android_cmake.sh .
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

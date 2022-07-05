#!/bin/bash
set -xe
source ./android_toolchain.sh $1 $2

ARTIFACT_LOCATION=$GITHUB_WORKSPACE


clone_scopy() {
	git clone $REPO_URL
	pushd scopy
	git submodule update --init --recursive iio-emu

	git fetch origin $BRANCH
	git checkout FETCH_HEAD
	popd
}


#cp libs
cp_scripts() {
	pushd scopy
	cp $BUILD_STATUS_FILE .
	cp $BUILD_ROOT/android_cmake.sh .
	cp ./CI/android/* .
	popd
}

make_apk_aab() {
	source ./android_toolchain.sh arm
	pushd scopy
	rm -rf build*

	./android_cmake.sh .
	cd $BUILDDIR
	make -j$JOBS iio-emu
	make -j$JOBS scopy
	cd ..
	./android_deploy_libs.sh
	./android_deploy_qt.sh apk

	pushd ../
	source ./android_toolchain.sh aarch64
	popd

	./android_cmake.sh .
	cd $BUILDDIR
	make -j$JOBS iio-emu
	make -j$JOBS scopy
	cd ..
	./android_deploy_libs.sh
	./android_deploy_qt.sh apk #sign

	./android_deploy_qt.sh aab #sign
	./android_get_symbols.sh
	popd
}



move_artifact() {
	pushd scopy
	sudo cp *.apk $ARTIFACT_LOCATION
	sudo cp *.aab $ARTIFACT_LOCATION
	sudo cp scopy*android-native-symbols.zip $ARTIFACT_LOCATION

	pushd $ARTIFACT_LOCATION
	sudo chmod 644 *.apk
	sudo chmod 644 *.aab
	sudo chmod 644 *.zip

	ls -la $ARTIFACT_LOCATION
	popd

	popd
}

 clone_scopy
 cp_scripts
 make_apk_aab
 move_artifact

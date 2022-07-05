#!/bin/bash
set -xe

if [ $# -ne 1 ]; then
        ARG1=${BUILDDIR}
else
        ARG1=$1
fi

copy-all-libs-from-staging() {
	echo -- Copying .so libraries to ./android/libs/$ABI
	cp $DEV_PREFIX/lib/*.so ./android/libs/$ABI
}
copy-missing-qt-libs() {
	echo -- Copying missing qt5 libraries to the android-build - for some reason android-qt-deploy does not correctly deploy all the libraries
	echo -- We are now deploying all the qt libraries - TODO only deploy the ones that are actually used
	cp $QT_INSTALL_PREFIX/lib/libQt5*_$ABI.so ./android/libs/$ABI
	#cp $QT_INSTALL_PREFIX/lib/libQt5*_$ABI.so $ARG1/android-build/libs/$ABI

}
copy-libsigrokdecode() {
	echo -- Copying libsigrokdecoders and python env
	cp -R $DEV_PREFIX/lib/python3.8 ./android/assets/
	cp -R $DEV_PREFIX/share/libsigrokdecode ./android/assets/
}

copy-iio-emu() {
	echo -- Deploying iio-emu as iio-emu.so in ./android/libs/$ABI
	echo -- this hack will deploy iio-emu with .so extension so the bundle tool
	echo -- will mark it as an executable
	echo -- surely there is a better way to do this

	cp $ARG1/iio-emu/iio-emu android/libs/$ABI/iio-emu.so
}
copy-scopy() {
	cp $ARG1/android-build/libs/$ABI/libscopy* android/libs/$ABI/
}

copy-all-libs-from-staging
copy-missing-qt-libs
copy-libsigrokdecode
copy-iio-emu
copy-scopy
#$ANDROID_QT_DEPLOY --input $ARG1/android_deployment_settings.json --output $ARG1/android-build --android-platform android-$API --jdk $JDK --gradle --verbose --sign /home/adi/Downloads/scopy-android-key.jks

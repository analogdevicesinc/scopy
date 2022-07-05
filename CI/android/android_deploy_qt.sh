#!/bin/bash
set -xe

source android_scopy_keystore

SCOPY_GIT_TAG=$(git rev-parse --short HEAD)
BUILD_TYPE="debug"
SIGN_STATUS="unsigned"
QT_DEPLOY_OUTPUT_FOLDER="./android-build"
SIGN_SUFFIX="${BUILD_TYPE}"

if [ "$2" == "--sign" ]; then
	BUILD_TYPE="release"
	SIGN_STATUS="signed"
	SIGN_OPTS="--sign ./scopy-android-key.jks scopy $KEYSTOREPASSWORD"
	SIGN_SUFFIX="${BUILD_TYPE}-${SIGN_STATUS}"
fi


if [ "$1" == "apk" ]; then
	DEPLOY_FILE_CMD="cp $QT_DEPLOY_OUTPUT_FOLDER/build/outputs/apk/${BUILD_TYPE}/*.apk ./scopy-${SCOPY_GIT_TAG}-${ABI}-${SIGN_SUFFIX}.${1}"
	# apk by default
	OUT_FILE_OPTS=""
elif [ "$1" == "aab" ]; then
	DEPLOY_FILE_CMD="cp $QT_DEPLOY_OUTPUT_FOLDER/build/outputs/bundle/${BUILD_TYPE}/*.aab ./scopy-${SCOPY_GIT_TAG}-${SIGN_SUFFIX}.${1} "
	OUT_FILE_OPTS="--aab"
else
	echo Required parameter -- $0 [apk/aab]
	exit
fi

init_android_build_folder() {
mkdir -p $QT_DEPLOY_OUTPUT_FOLDER
cp -R ${BUILDDIR}/android-build/* $QT_DEPLOY_OUTPUT_FOLDER/
}

android_deploy() {
$ANDROID_QT_DEPLOY \
	--input ${BUILDDIR}/android_deployment_settings.json \
	--output ${QT_DEPLOY_OUTPUT_FOLDER} \
	--android-platform android-${API} \
	--jdk ${JDK} \
	--gradle \
	--verbose ${SIGN_OPTS} \
	${OUT_FILE_OPTS} \
	$SIGN_OPTS \

${DEPLOY_FILE_CMD}
}

init_android_build_folder
android_deploy
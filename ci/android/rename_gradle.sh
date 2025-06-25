#!/bin/bash
set -xe
source ./android_toolchain.sh

rename_gradle() {
	echo "Patching to use gradle-7.4"

	pushd $QT_LOCATION/src/3rdparty/gradle/gradle/wrapper
	sed -i "s|https\\\:\/\/services\.gradle\.org\/distributions\/gradle.*\.zip|https\://services.gradle.org/distributions/gradle-7.4-all.zip|g" gradle-wrapper.properties
	popd
}

patch_sdk_build_tools_revision() {
	echo "Patching qt android to support sdkBuildToolsRevision"
	# https://bugreports.qt.io/browse/QTBUG-84302

	pushd $QT_LOCATION/lib/cmake/Qt5Core/
	cp Qt5AndroidSupport.cmake Qt5AndroidSupport.cmake.backup
	sed '59 a "sdkBuildToolsRevision": "@ANDROID_SDK_BUILD_TOOLS_REVISION@",' Qt5AndroidSupport.cmake.backup  > Qt5AndroidSupport.cmake
	rm -rf Qt5AndroidSupport.cmake.backup
	popd
}

rename_gradle
patch_sdk_build_tools_revision

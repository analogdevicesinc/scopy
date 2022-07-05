#!/bin/bash
SCOPY_GIT_TAG=$(git rev-parse --short HEAD)
SYMBOLS_FOLDER=android-build/build/intermediates/merged_native_libs/release/out/lib/
ARCHIVE_NAME=scopy-$SCOPY_GIT_TAG-android-native-symbols.zip

pushd $SYMBOLS_FOLDER
zip -r $ARCHIVE_NAME *
popd
mv ${SYMBOLS_FOLDER}/${ARCHIVE_NAME} .

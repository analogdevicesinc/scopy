#!/usr/bin/env bash

## Set STAGING
USE_STAGING=OFF

echo "Navigating to the package generator directory..."
cd /home/docker/scopy/scopy/tools/packagegenerator

echo "Installing Python dependencies..."
pip3 install -r requirements.txt

echo "Testing the package generator script..."
python3 ./package_generator.py -h

echo "Creating package directory for the repository..."
mkdir -p /home/docker/scopy/scopy/packages/$GITHUB_REPOSITORY

echo "Copying package files to the repository directory..."
cp -r /home/docker/scopy/package/* /home/docker/scopy/scopy/packages/$GITHUB_REPOSITORY/

echo "Building scopy.."
cd /home/docker/scopy/scopy

echo "Creating and navigating to the build directory..."
mkdir -p build && cd build

source /home/docker/scopy/scopy/ci/windows/mingw_toolchain.sh $USE_STAGING

$CMAKE .. $RC_COMPILER_OPT -DPYTHON_EXECUTABLE=$STAGING_DIR/bin/python3.exe -DENABLE_TESTING=OFF
$MAKE_BIN $JOBS

echo "Returning to the package generator directory..."
cd /home/docker/scopy/scopy/tools/packagegenerator

echo "Running the package generator script to create the final artifact..."
$STAGING_DIR/bin/python3.exe ./package_generator.py -a --src=/home/docker/scopy/scopy/build/packages/$GITHUB_REPOSITORY --dest=/home/docker/artifact_x86_64

echo "Build and packaging process completed successfully!"

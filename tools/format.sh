#!/bin/bash
set -x

SCOPY_REPO=$(git rev-parse --show-toplevel)

IN_PLACE="-i"
VERBOSE="--verbose"
CLANG_FORMAT="clang-format-10"
CMAKE_FORMAT="cmake-format"

find $SCOPY_REPO -type f -name "*.cpp" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO -type f -name "*.cc" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO -type f -name "*.hpp" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO -type f -name "*.h" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO -type f -name "CMakeLists.txt" | xargs $CMAKE_FORMAT $IN_PLACE
find $SCOPY_REPO -type f -name "*.cmake" | xargs $CMAKE_FORMAT $IN_PLACE
pushd $SCOPY_REPO/tools
./includemocs.sh
popd

echo Formatting complete


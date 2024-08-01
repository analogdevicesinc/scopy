#!/bin/bash
set -x

SCOPY_REPO=$(git rev-parse --show-toplevel)
SKIP_BUILD="-not -path */build/* -not -path */ci/flatpak*"

IN_PLACE="-i"
VERBOSE="--verbose"
CLANG_FORMAT="clang-format-12"
CMAKE_FORMAT="cmake-format"

find $SCOPY_REPO $SKIP_BUILD -type f -name "*.cpp" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO $SKIP_BUILD -type f -name "*.cc" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO $SKIP_BUILD -type f -name "*.hpp" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO $SKIP_BUILD -type f -name "*.h" | xargs $CLANG_FORMAT $VERBOSE $IN_PLACE
find $SCOPY_REPO $SKIP_BUILD -type f -name "CMakeLists.txt" | xargs $CMAKE_FORMAT $IN_PLACE
find $SCOPY_REPO $SKIP_BUILD -type f -name "*.cmake" | xargs $CMAKE_FORMAT $IN_PLACE
pushd $SCOPY_REPO/tools
./includemocs.sh
popd

echo Formatting complete


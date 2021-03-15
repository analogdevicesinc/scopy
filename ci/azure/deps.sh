#!/bin/bash

set -ex

. ci/azure/lib.sh

deps_default() {
   echo "default"
}

deps_clang_format() {
    sudo apt-get install clang-format-11
    echo_green "Clang-format version: " `/usr/bin/clang-format-11 --version`
}

deps_${BUILD_TYPE:-default}

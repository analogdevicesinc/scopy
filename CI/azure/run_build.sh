#!/bin/bash

set -ex

. CI/azure/lib.sh

build_default() {
    echo "default"
}

build_clang_format() {
    check_clangformat
}

build_cmake_format() {
    check_cmakeformat
}

build_${BUILD_TYPE:-default}

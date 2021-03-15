#!/bin/bash

set -ex

. ci/azure/lib.sh

build_default() {
    echo "default"
}

build_clang_format() {
    check_clangformat
}

build_${BUILD_TYPE:-default}

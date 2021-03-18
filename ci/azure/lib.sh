#!/bin/bash

echo_red() { printf "\033[1;31m$*\033[m\n"; }
echo_green() { printf "\033[1;32m$*\033[m\n"; }

############################################################################
# Check if the file passed as arguments is ignored or not by clang format
############################################################################
is_not_ignored() {
    local file="$1"

    fileData=`cat .clangformatignore`

    for entry in $fileData; do
        if [ -d "${entry}" ]; then
            pushd ${entry}
            fileName=`basename ${file}`
            found=$(find -name ${fileName} | wc -l)
            if [ ${found} -gt 0 ]; then
                popd
                return 1
            else
                popd
            fi
        else
            if [ -f "${entry}" ]; then
                if [ "${file}" == "${entry}" ]; then
                    return 1
                fi
            fi
        fi
    done;
    return 0
}


############################################################################
# Check if the file given as input has .h or .cpp extension
############################################################################
is_source_file() {
    local file="$1"

    EXTENSIONS=".h .hpp .cpp"

    for extension in $EXTENSIONS; do
        [[ "${file: -2}" == "$extension" || "${file: -4}" == "$extension" ]] && return 0
    done;

    return 1
}

############################################################################
# Check if the files modified in the current commit / commit range respect
# the coding style defined in the .clang-format file
############################################################################
check_clangformat() {

    if [ -z "$TRAVIS_PULL_REQUEST_SHA" ]
    then
        COMMIT_RANGE=HEAD~1
    fi

    # git diff --name-only --diff-filter=d $COMMIT_RANGE | while read -r file; do
    git ls-tree -r --name-only HEAD | while read -r file; do
        if is_source_file "$file" && is_not_ignored "$file"
        then
            /usr/bin/clang-format-11 -i "$file"
        fi

    done;

    git diff --exit-code || {
        echo_red "The code is not properly formatted."

        mkdir -p ${ARTIFACTS_DIR}
        pushd ${ARTIFACTS_DIR}

        touch format-code.patch
        git diff > format-code.patch

        popd # pushd ${ARTIFACTS_DIR}

        echo_red "The code style issues can be fixed by applying the format-code.patch found in the artifacts sections"
        echo_red "The command to apply the patch is: git apply format-code.patch"

        exit 1
    }
}

check_cmakeformat() {
    find . -type f -iname CMakeLists.txt ! -path "*old*" -o -type f -iname "*.cmake" ! -path "*old*" | xargs cmake-format -i

    git diff --exit-code || {
        echo_red "The cmake files are not properly formatted."

        mkdir -p ${ARTIFACTS_DIR}
        pushd ${ARTIFACTS_DIR}

        touch format-cmake.patch
        git diff > format-cmake.patch

        popd # pushd ${ARTIFACTS_DIR}

        echo_red "The cmake style issues can be fixed by applying the format-cmake.patch found in the artifacts sections"
        echo_red "The command to apply the patch is: git apply format-cmake.patch"

        exit 1
    }
}

#!/bin/bash

set -e

usage()
{
	echo "test"

}
COPY=""

while [ "$1" != "" ]
do
    case "$1" in
        "--help"| "-h")
            usage
            exit
            ;;
        "--lib-dir" | "-l")
            SYSROOT="$2:$SYSROOT"
            shift
            ;;
        "--output-dir" | "-o")
            OUTPUT_DIR="$2"
            COPY="true"
            shift
            ;;
        *)
            FILES="$FILES $1"
            ;;
    esac
    shift
done

LIBS_ARRAY=()
# Separate SYSROOT into an array of SEARCH_PATHS
IFS=":" read -r -a SEARCH_PATHS <<< "$SYSROOT"

# If COPY is not true, display all dependencies without excluding any from the blacklist
if [[ $COPY != "true" ]]; then
    BLACKLISTED=""
else
    BLACKLISTED=($(wget --quiet https://raw.githubusercontent.com/probonopd/AppImages/master/excludelist -O - | sort | uniq | cut -d '#' -f 1 | grep -v "^#.*" | grep "[^-\s]"))
    BLACKLISTED+=(ld-linux-armhf.so)
    BLACKLISTED+=(ld-linux-aarch64.so)
    BLACKLISTED+=(ld-linux-armhf.so.3)
    BLACKLISTED+=(ld-linux-aarch64.so.1)
fi

findlib()
{
    LIB=$1
    for path in "${SEARCH_PATHS[@]}"
    do
        LIB_PATH=$(find "$path" -name "${LIB%%.so*}.so*" -type f  2>/dev/null | sort | head -1)
        [ "$LIB_PATH" != "" ] && break # Exit for at first succesful find
    done

    [ "$LIB_PATH" = "" ] && LIB_PATH="not found"
    echo "$LIB_PATH"
}

copylib()
{
    library=$1
    destination=$2

    lib_location=${library%/*}
    lib_name=${library##*/}
    lib_name_without_version=${lib_name%%.so*}.so

    echo "${lib_name_without_version}"
    echo -n copied:
    cp --verbose "${library}" "${destination}"

    #IFS=$'\n' read -r -a symlinks < <(find "$lib_location" -name "${lib_name_without_version}*" -type l 2>/dev/null) || echo "No symlinks to copy"

    symlinks=($(find "$lib_location" -name "${lib_name_without_version}*" -type l 2>/dev/null))
    for link in "${symlinks[@]}"
    do
       pushd ${destination} > /dev/null
       [ -L "${link##*/}" ] && rm "${link##*/}"
       echo -n linked:
       ln --symbolic --verbose "${library##*/}" "${link##*/}"
       popd > /dev/null
    done
}

process_lib(){
    DEPS=$(readelf -d "$1"  2> /dev/null | \
                grep NEEDED | \
                cut -d ":" -f 2 | \
                sed -e 's,\[,,g' -e 's,],,g' -e 's,[ ]*,,g'| \
                sort)

    for library in $DEPS
    do
        # Check if the library is blacklisted or was already processed
        if  [[ ! "${BLACKLISTED[*]}" =~ "${library}"  && ! "${LIBS_ARRAY[*]}" =~ "${library}" ]]; then

        LIBS_ARRAY+=("${library}")
        LIB_PATH=$(findlib "$library") # try to find the library in sysroot

        if [[ $COPY == "true" ]]; then
            if [[ $LIB_PATH == "not found" ]]; then
                echo "Error: ${library} not found"
                exit 1
            fi
            mkdir -p "$OUTPUT_DIR"
            copylib "$LIB_PATH" "$OUTPUT_DIR"
        fi

        # If COPY is not true, display only the library and its location if found
        [ "$COPY" != "true" ] && echo "$library => $LIB_PATH"
        process_lib "$LIB_PATH"
        fi
    done
}

for arg in $FILES; do
    process_lib "${arg}"
done
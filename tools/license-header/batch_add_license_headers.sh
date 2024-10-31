#!/bin/bash

# Usage: process_files.sh <file_list.txt> [options]

# Global variables
TEMPLATE_FILE=""
PARAMS_FILE=""

function usage() {
    cat <<EOF
    Usage: process_files.sh <file_list.txt> [options]
    Process a list of files to add license headers.

    Options:
        --template TEMPLATE_FILE_PATH       Path to the template file.
        --params PARAMS_FILE_PATH           Path to the parameters file. If not provided, it is assumed to be located in the same directory as the template file.
                                            Default: template_directory/params.conf
        -h, --help                          Display this help message.

EOF
}

function parse_arguments() {
    LONG_OPTS=template:,params:,help
    OPTIONS=t:p:h
    VALID_ARGS=$(getopt --options=$OPTIONS --longoptions=$LONG_OPTS --name "$0" -- "$@")

    if [[ $? -ne 0 ]]; then
        usage
        exit 1
    fi

    eval set -- "$VALID_ARGS"

    while true; do
        case "$1" in
        -t | --template)
            TEMPLATE_FILE="$2"
            shift 2
            ;;
        -p | --params)
            PARAMS_FILE="$2"
            shift 2
            ;;
        -h | --help)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Unknown argument: $1"
            usage
            exit 1
            ;;
        esac
    done

    if [[ $# -lt 1 ]]; then
        echo "Error: No file list provided."
        usage
        exit 1
    fi

    FILE_LIST="$1"
}

function process_files() {
    local file_list="$1"
    local add_license_header="./add_license_header.sh"

    while IFS= read -r file; do
        # Trim trailing whitespaces
        file=$(echo "$file" | sed 's/[[:space:]]*$//')

        [[ -z "$file" || "$file" =~ ^# ]] && continue

        # Call add_license_header.sh with the provided options
        "$add_license_header" "$file" \
            --template "$TEMPLATE_FILE" \
            --params "$PARAMS_FILE"
    done <"$file_list"
}

function main() {
    set -euo pipefail

    parse_arguments "$@"

    # Check if TEMPLATE_FILE is provided
    if [[ -z "$TEMPLATE_FILE" ]]; then
        echo "Error: No template file provided."
        usage
        exit 1
    fi

    process_files "$FILE_LIST"
    exit 0
}

main "$@"

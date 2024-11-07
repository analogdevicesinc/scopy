#!/bin/bash

# Usage: add_header.sh <file>
#                      [--template TEMPLATE_FILE]
#                      [--params PARAMS_FILE]

# Global variables
TEMPLATE_FILE=""
PARAMS_FILE=""

function usage() {
    cat <<EOF
    Usage: add_header.sh <file> [options]
    Add a license header to a file.

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

    getopt --test >/dev/null && true
    if [[ $? -ne 4 ]]; then
        echo "$(getopt --test) failed in this environment."
        exit 1
    fi

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
        echo "Error: No file provided."
        usage
        exit 1
    fi

    FILE="$1"
}

function load_comment_styles() {
    local COMMENT_STYLES_FILE="./.comment_styles.conf"
    declare -gA COMMENT_STYLES

    while IFS=: read -r ext style; do
        # Skip empty lines or lines starting with #
        [[ -z "$ext" || "$ext" =~ ^# ]] && continue
        COMMENT_STYLES["$ext"]="$style"
    done <"$COMMENT_STYLES_FILE"
}

function load_parameters() {
    declare -gA PARAMS

    # Is PARAMS_FILE not provided? Assume it's in the template directory
    if [[ -z "$PARAMS_FILE" ]]; then
        PARAMS_FILE="$(dirname "$TEMPLATE_FILE")/params.conf"
    fi

    if [[ -f "$PARAMS_FILE" ]]; then
        while IFS='=' read -r key value; do
            # Skip empty lines or lines starting with #
            [[ -z "$key" || "$key" =~ ^# ]] && continue

            # Check if the value starts with 'eval:'
            if [[ "$value" == eval:* ]]; then
                # Remove the 'eval:' prefix
                command_to_eval="${value#eval:}"
                value=$(eval "$command_to_eval")
                if [[ $? -ne 0 ]]; then
                    echo "Error: Failed to evaluate command: $command_to_eval" >&2
                    exit 1
                fi
            fi
            PARAMS["$key"]="$value"
        done <"$PARAMS_FILE"
    # TODO: what if the license has no temple??
    # else
    #     echo "Error: Parameters file '$PARAMS_FILE' not found."
    #     exit 1
    fi
}

function replace_placeholders() {
    local template_content="$1"
    local placeholders

    # Extract placeholders from the template
    placeholders=$(grep -o '{{[^}]*}}' "$TEMPLATE_FILE" | sort | uniq)

    for placeholder in $placeholders; do
        local key="${placeholder//\{\{/\}}"
        key="${key//\}\}/}"

        key=$(echo "$key" | sed 's/[{}]*//g')
        value="${PARAMS[$key]:-}"

        if [[ -z "$value" ]]; then
            echo "Warning: No value provided for placeholder $placeholder"
            value=""
        fi

        # Escape special characters
        local escaped_value
        escaped_value=$(printf '%s' "$value" | sed 's/[\/&]/\\&/g')
        template_content=$(echo "$template_content" | sed "s/$placeholder/$escaped_value/g")
    done

    echo "$template_content"
}

function adapt_comment_style() {
    local comment_style=$1
    local input_text=$2
    local formatted_text=""

    case "$comment_style" in
    "c_style")
        formatted_text="/*\n$(echo "$input_text" | sed 's/^/ * /; s/^ * $/ */')\n *\n */\n"
        ;;
    "hash_style")
        formatted_text="#\n$(echo "$input_text" | sed 's/^/# /; s/^# $/#/')\n#\n"
        ;;
    "slash_style")
            formatted_text="// \n$(echo "$input_text" | sed 's/^/\/\/ /; s/^\/\/ $/\/\//')\n// \n"
            ;;
    # "bracket_style")
    #     formatted_text="(*\n$(echo "$input_text" | sed 's/^/ * /; s/^ * $/ *)/')\n"
    #     ;;
    *)
        echo "Unsupported comment style: $comment_style" >&2
        exit 1
        ;;
    esac

    # Remove trailing whitespace
    formatted_text=$(echo "$formatted_text" | sed 's/[[:space:]]*$//')
    echo -e "$formatted_text"
}

function insert_license_sh() {
    local file="$1"
    local license_text="$2"
    local tmp_file
    tmp_file=$(mktemp)
    
    first_line=$(head -n 1 "$file")
    
    {
        echo "$first_line"
        echo "$license_text"
        echo ""
        tail -n +2 "$file"
    } >"$tmp_file"
    
    mv "$tmp_file" "$file"
}

function insert_license_default() {
    local file="$1"
    local license_text="$2"
    local tmp_file
    tmp_file=$(mktemp)
    
    {
        echo "$license_text"
        echo ""
        cat "$file"
    } >"$tmp_file"
    
    mv "$tmp_file" "$file"
}

# Inserts the formatted license at the top of the file
function insert_license() {
    local file="$1"
    local license_text="$2"
    local file_extension="$3"
    
    if [ "$file_extension" = "sh" ]; then
        insert_license_sh "$file" "$license_text"
    else
        insert_license_default "$file" "$license_text"
    fi
}

function main() {
    set -eou pipefail

    parse_arguments "$@"

    # Check if TEMPLATE_FILE is provided
    if [[ -z "$TEMPLATE_FILE" ]]; then
        echo "Error: No template file provided."
        usage
        exit 1
    fi

    # Supported comment styles used in the license header
    load_comment_styles
    
    FILE_EXTENSION="${FILE##*.}"

    COMMENT_STYLE=${COMMENT_STYLES[$FILE_EXTENSION]:-}
    if [[ -z "$COMMENT_STYLE" ]]; then
        echo "Error: Unsupported file extension '${FILE_EXTENSION}' found in file '${FILE}'" >&2
        echo "Define 'COMMENT_STYLE' for '$FILE_EXTENSION' in .comment_styles.conf" >&2
        exit 1
    fi

    # Read the template file
    if [[ -f "$TEMPLATE_FILE" ]]; then
        LICENSE_CONTENT=$(<"$TEMPLATE_FILE")
    else
        echo "Error: Template file '$TEMPLATE_FILE' not found."
        exit 1
    fi

    # Parameters used to replace placeholders in the template
    load_parameters

    # Replace placeholders
    LICENSE_CONTENT=$(replace_placeholders "$LICENSE_CONTENT")

    # Adapt the license text to the comment style
    FORMATTED_LICENSE=$(adapt_comment_style "$COMMENT_STYLE" "$LICENSE_CONTENT")

    insert_license "$FILE" "$FORMATTED_LICENSE" "$FILE_EXTENSION"
}

main "$@"

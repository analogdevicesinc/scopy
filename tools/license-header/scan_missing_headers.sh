#!/bin/bash

# set -x

# Initialize global variables
OMITTED_DIRS=()
OMITTED_FILES=()
BASE_DIR_PATH=""
FILES_WITHOUT_LICENSE=()
FILE_EXTENSIONS=()
N_LINES=80 # When searching for license headers, use the first N lines of the file
VERBOSE=false

# Define colors used for logging
NC='\033[0m'         # No Color
GREEN='\033[0;32m'   # Files with LGPL license
BLUE='\033[0;34m'    # Files with GPL license
YELLOW='\033[1;33m'  # Files with ADI-BSD license
RED='\033[0;31m'     # Files with no license header
MAGENTA='\033[0;35m' # Files with unknown license header

usage() {
    cat <<EOF
Usage: license_scanner.sh --path /path/to/directory [options]
Scan files for licenses.
Options:
    -d, --dirs          Comma-separated list of directory names to omit.
                        Example: license_scanner.sh --dirs=build,examples

    -f, --files         Comma-separated list of file names to omit.
                        Example: license_scanner.sh --files=LICENSE,README.md

    -p, --path          Path of the base directory to scan.
                        Example: license_scanner.sh --path /path/to/directory

    -l, --lines         Number of lines to search for license headers.
                        Default: 80

    -h, --help          Display this help message.

    -v, --verbose       Display verbose output.

EOF
}

function parse_arguments() {
    LONG_OPTS=dirs:,files:,path:,help,verbose
    OPTIONS=d:f:p:h:v
    VALID_ARGS=$(getopt --options=$OPTIONS --longoptions=$LONG_OPTS --name "$0" -- "$@")

    if [[ $? -ne 0 ]]; then
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
        -d | --dirs)
            shift
            IFS="," read -ra OMITTED_DIRS <<<"$1"
            shift
            ;;
        -f | --files)
            shift
            IFS="," read -ra OMITTED_FILES <<<"$1"
            shift
            ;;
        -h | --help)
            usage
            exit 0
            ;;
        -l | --lines)
            N_LINES="$2"
            shift 2
            ;;
        -p | --path)
            BASE_DIR_PATH="$2"
            shift 2
            ;;
        -v | --verbose)
            VERBOSE=true
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Invalid option: -$OPTARG" >&2
            exit 2
            ;;
        esac
    done
}

function validate_base_path() {
    if [[ ! -d "$BASE_DIR_PATH" ]]; then
        echo "Error: The base directory path is invalid." >&2
        exit 3
    fi
}

function read_header() {
    local file="$1"
    local n_lines="$2"
    head -n "$n_lines" "$file"
}

function test_regex() {
    local content="$1"
    local regex="$2"

    if echo "$content" | grep --quiet --ignore-case --extended-regexp "$regex"; then
        return 0
    else
        return 1
    fi
}

function normalize_text() {
    # Remove comment characters and leading/trailing whitespaces from lines that start with a comment
    sed -E 's/^[[:space:]]*(#|\/\/|\/\*|\*|<!--)[[:space:]]*//; s/[[:space:]]*$//' |
        # Replace multiple spaces with a single space
        tr -s '[:space:]' ' ' |
        # Trim leading/trailing spaces
        sed 's/^ *//;s/ *$//'
}

function is_LGPL() {
    local header="$1"
    local lgpl_patterns=(
        "GNU\s*Lesser\s*General\s*Public\s*License" # Standard LGPL reference
        "LGPL"                                      # Short form LGPL
        "LGPLv[23](\.[01])?"                        # Specific verion of LGPL
        "Lesser\s*General\s*Public\s*License,\s*Version\s*[23](\.[01])?"
    )
    # Match any of the patterns
    for pattern in "${lgpl_patterns[@]}"; do
        if test_regex "$header" "$pattern"; then
            return 0
        fi
    done
    return 1
}

function is_GPL() {
    local header="$1"
    local gpl_patterns=(
        "GNU\s*General\s*Public\s*License" # Standard GPL reference
        "GPL"                              # Short form GPL
    )
    # Match any of the patterns
    for pattern in "${gpl_patterns[@]}"; do
        if test_regex "$header" "$pattern"; then
            return 0
        fi
    done
    return 1
}

function is_Scopy_GPL() {
    local header="$1"
    local scopy_license_regex='Copyright \(c\) \b[0-9]{4}\b Analog Devices Inc. This file is part of Scopy \(see http[s]?://www.github.com/analogdevicesinc/scopy\). This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or \(at your option\) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY\; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http[s]?://www.gnu.org/licenses/>.'
    test_regex "$header" "$scopy_license_regex"
}

# TODO: normalize header and adapt pattern
function is_ADI_BSD() {
    local header="$1"
    local adi_bsd_patterns=(
        "THIS\s*SOFTWARE\s*IS\s*PROVIDED\s*BY\s*ANALOG\s*DEVICES\s*\"?AS\s*IS\"?"
        "IMPLIED\s*WARRANTIES,\s*INCLUDING,\s*BUT\s*NOT\s*LIMITED\s*TO,\s*NON-INFRINGEMENT"
        "MERCHANTABILITY\s*AND\s*FITNESS\s*FOR\s*A\s*PARTICULAR\s*PURPOSE\s*ARE\s*DISCLAIMED"
        "IN\s*NO\s*EVENT\s*SHALL\s*ANALOG\s*DEVICES\s*BE\s*LIABLE\s*FOR\s*ANY\s*DIRECT,\s*INDIRECT"
        "INCLUDING,\s*BUT\s*NOT\s*LIMITED\s*TO,\s*INTELLECTUAL\s*PROPERTY\s*RIGHTS"
        "LOSS\s*OF\s*USE,\s*DATA,\s*OR\s*PROFITS;\s*OR\s*BUSINESS\s*INTERRUPTION"
        "WHETHER\s*IN\s*CONTRACT,\s*STRICT\s*LIABILITY,\s*OR\s*TORT\s*\(INCLUDING\s*NEGLIGENCE\)"
        "EVEN\s*IF\s*ADVISED\s*OF\s*THE\s*POSSIBILITY\s*OF\s*SUCH\s*DAMAGE"
    )
    # Match all of the patterns
    for pattern in "${adi_bsd_patterns[@]}"; do
        if ! test_regex "$header" "$pattern"; then
            return 1
        fi
    done
    return 0
}

function is_GNU_radio() {
    local header="$1"
    local gr_patterns=(
        "This\s*file\s*is\s*part\s*of\s*GNU\s*Radio"
    )
    # Match any of the patterns
    for pattern in "${gr_patterns[@]}"; do
        if test_regex "$header" "$pattern"; then
            return 0
        fi
    done
    return 1
}

function has_ADI_disclaimer() {
    local header="$1"
    local adi_disclaimer_patterns=(
        "Analog\s*Devices\s*Inc\."
    )
    # Match any of the patterns
    for pattern in "${adi_disclaimer_patterns[@]}"; do
        if test_regex "$header" "$pattern"; then
            return 0
        fi
    done
    return 1
}

function has_license_disclaimer() {
    local header="$1"
    local license_patterns=(
        "license"
        "copyright"
        "disclaimer"
        "reserved"
    )
    # Match any of the patterns
    for pattern in "${license_patterns[@]}"; do
        if test_regex "$header" "$pattern"; then
            return 0
        fi
    done
    return 1
}

function identify_license() {
    local file=$1
    local n_lines=$2
    local relative_path
    relative_path=$(realpath --relative-to="$BASE_DIR_PATH" "$file")

    local depth indent
    depth=$(echo "$relative_path" | awk -F'/' '{print NF-1}')
    indent=$(printf "%*s" $((depth * 4)) "")

    local header=$(read_header "$file" "$n_lines" | normalize_text) 

    ((total_checks++))

    if ! has_license_disclaimer "$header"; then
        ((no_license_count++))
        FILES_WITHOUT_LICENSE+=("$relative_path")

        if $VERBOSE; then
            echo -e "${indent}${RED}├── $relative_path (No License)${NC}"
        fi
        # Stop execution if no license is found
        return 
    fi

    local found_licenses_list=()
    local color=$MAGENTA

    local unknown=true

    # Search for license headers

    if has_ADI_disclaimer "$header"; then
        if is_GPL "$header"; then
            if is_Scopy_GPL "$header"; then
                ((scopy_gpl_count++))
                unknown=false
                color=$GREEN
                found_licenses_list+=("Scopy_GPL")
            elif is_LGPL "$header"; then
                ((lgpl_count++))
                unknown=false
                color=$GREEN
                found_licenses_list+=("LGPL")
            else
                ((gpl_count++))
                unknown=false
                color=$BLUE
                found_licenses_list+=("GPL")
            fi
        fi
        if is_ADI_BSD "$header"; then
            ((adi_bsd_count++))
            unknown=false
            color=$YELLOW
            found_licenses_list+=("ADI-BSD")
        fi
    fi

    if is_GNU_radio "$header"; then
        ((gr_count++))
        unknown=false
        color=$YELLOW
        found_licenses_list+=("GNU Radio")
    fi

    if $unknown; then
        ((unknown_license_count++))
    fi

    if $VERBOSE; then
        # Join the elements of found_licenses_list with a | separator
        local joined_licenses=$(
            IFS='|'
            echo "${found_licenses_list[*]}"
        )

        # Check if the joined string is empty
        if [ -z "$joined_licenses" ]; then
            joined_licenses="Unknown"
        fi

        # Update the echo command to use the joined string
        echo -e "${indent}${color}├── $relative_path (${joined_licenses})${NC}"
    fi
}


function scan_directory() {
    local BASE_DIR_PATH=$1
    local dir_excludes file_excludes
    dir_excludes=()
    file_excludes=()

    # Use the --build option to exclude directories
    if [ ${#OMITTED_DIRS[@]} -gt 0 ]; then
        for dir in "${OMITTED_DIRS[@]}"; do
            dir_excludes+=(-o -iname "$dir")
        done
        dir_excludes=(-type d \( -iname "${dir_excludes[@]:2}" \) -prune)
    fi
    # Use the --files option to exclude figles
    if [ ${#OMITTED_FILES[@]} -gt 0 ]; then
        for file in "${OMITTED_FILES[@]}"; do
            file_excludes+=(-o -iname "$file")
        done
        file_excludes=(-type f ! \( -iname "${file_excludes[@]:2}" \))
    fi

    # Build the find command
    find_cmd=(find "$BASE_DIR_PATH")

    if [ ${#dir_excludes[@]} -gt 0 ]; then
        find_cmd+=("${dir_excludes[@]}")
    fi

    if [ ${#file_excludes[@]} -gt 0 ]; then
        if [ ${#dir_excludes[@]} -gt 0 ]; then
            find_cmd+=(-o)
        fi
        find_cmd+=("${file_excludes[@]}")
    else
        find_cmd+=(-o)
    fi

    find_cmd+=(-print)

    "${find_cmd[@]}"
}

function main() {
    parse_arguments "$@"
    validate_base_path

    total_checks=0
    no_license_count=0
    lgpl_count=0
    scopy_gpl_count=0
    gpl_count=0
    adi_bsd_count=0
    gr_count=0
    unknown_license_count=0

    # Store the number of unique file extensions
    if $VERBOSE; then
        # Find all files recursively in the base directory and extract unique file extensions
        mapfile -t FILE_EXTENSIONS < <(scan_directory "$BASE_DIR_PATH" -type f | awk -F. '{if (NF>1) print $NF}' | sort -u)
        echo "Unique file extensions: ${FILE_EXTENSIONS[*]}"
    fi

    while read -r file; do
        if [[ -f "$file" ]]; then
            identify_license "$file" "$N_LINES"
        fi
    done < <(scan_directory "$BASE_DIR_PATH" | sort)

    if $VERBOSE; then
        echo -e "##################################################"
        echo -e "License count summary:"
        echo -e "LGPL : [$lgpl_count/$total_checks]"
        echo -e "Scopy_GPL: [$scopy_gpl_count/$total_checks]"
        echo -e "GPL: [$gpl_count/$total_checks]"
        echo -e "ADI-BSD: [$adi_bsd_count/$total_checks]"
        echo -e "GNU Radio: [$gr_count/$total_checks]"
        echo -e "No-license: [$no_license_count/$total_checks]"
        echo -e "Unknown-license: [$unknown_license_count/$total_checks]\n"
        echo "Scanning project for license headers from path: $BASE_DIR_PATH ..." >&2
    fi

    if [ ${#FILES_WITHOUT_LICENSE[@]} -gt 0 ]; then
        for file in "${FILES_WITHOUT_LICENSE[@]}"; do
            echo -e "$BASE_DIR_PATH/$file"
        done
        exit 1
    fi

    echo "All files have a license."
    if [ "$total_checks" -ne "$scopy_gpl_count" ]; then
        echo "Error: Not all files have the Scopy GPL license."
        exit 1
    fi
    exit 0
}

main "$@"

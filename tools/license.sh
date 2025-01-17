#!/bin/bash

# set -x

CI_SCRIPT=${CI_SCRIPT:-"OFF"}

REPOSTORY=$(git rev-parse --show-toplevel)
PATH_OPTION=" --path $REPOSTORY"
LICENSE_UTILS_PATH=$REPOSTORY/tools/license-header
RESULTS_PATH=$LICENSE_UTILS_PATH/results

source $REPOSTORY/tools/exclude_list.sh

# Build options
OMIT_DIRS_OPTION=""
if [ ${#OMIT_DIRS_LIST[@]} -gt 0 ] && [ ${#GITIGNORE_DIRS_LIST[@]} -gt 0 ]; then
    OMIT_DIRS_OPTION=" --dirs $(
        IFS=,
        echo "${OMIT_DIRS_LIST[*]},${GITIGNORE_DIRS_LIST[*]}"
    )"
elif [ ${#OMIT_DIRS_LIST[@]} -gt 0 ]; then
    OMIT_DIRS_OPTION=" --dirs $(
        IFS=,
        echo "${OMIT_DIRS_LIST[*]}"
    )"
elif [ ${#GITIGNORE_DIRS_LIST[@]} -gt 0 ]; then
    OMIT_DIRS_OPTION=" --dirs $(
        IFS=,
        echo "${GITIGNORE_DIRS_LIST[*]}"
    )"
fi

OMIT_FILES_OPTION=""
if [ ${#OMIT_FILES_LIST[@]} -gt 0 ] && [ ${#GITINORE_FILES_LIST[@]} -gt 0 ]; then
    OMIT_FILES_OPTION=" --files $(
        IFS=,
        echo "${OMIT_FILES_LIST[*]},${GITINORE_FILES_LIST[*]}"
    )"
elif [ ${#OMIT_FILES_LIST[@]} -gt 0 ]; then
    OMIT_FILES_OPTION=" --files $(
        IFS=,
        echo "${OMIT_FILES_LIST[*]}"
    )"
elif [ ${#GITINORE_FILES_LIST[@]} -gt 0 ]; then
    OMIT_FILES_OPTION=" --files $(
        IFS=,
        echo "${GITINORE_FILES_LIST[*]}"
    )"
fi

main() {
    mkdir -p "$RESULTS_PATH"
    [ -f "$RESULTS_PATH/scan_output.txt" ] && rm "$RESULTS_PATH/scan_output.txt"
    touch "$RESULTS_PATH"/scan_output.txt

    [ -f "$RESULTS_PATH/log.txt" ] && rm "$RESULTS_PATH/log.txt"
    touch "$RESULTS_PATH"/log.txt

    echo "# Running license scanner utility ..."
    $LICENSE_UTILS_PATH/scan_missing_headers.sh \
        $PATH_OPTION \
        $OMIT_DIRS_OPTION \
        $OMIT_FILES_OPTION \
        >$RESULTS_PATH/scan_output.txt 2>$RESULTS_PATH/log.txt
    EXIT_CODE=$?

    echo "## Missing header scan paths: $RESULTS_PATH/scan_output.txt"
    echo "## Missing headers (detailed logs): $RESULTS_PATH/log.txt"

    # CI should fail if missing headers are detected
    if [ "$CI_SCRIPT" == "ON" ]; then
        echo "## CI mode: Exiting with scan results"
        echo "## Log results:"
        cat "$RESULTS_PATH"/log.txt
        echo "## Report results:"
        cat "$RESULTS_PATH"/scan_output.txt

        if [ $EXIT_CODE -ne 0 ]; then
            echo "### Error: Missing headers found"
        else
            echo "### Success: All scanned files contain license headers"
        fi
        exit $EXIT_CODE
    fi

    if [ $EXIT_CODE -ne 0 ]; then
        pushd "$LICENSE_UTILS_PATH" || exit 1 >/dev/null
        echo "### Warning: Missing headers found"
        echo "# Adding missing headers ..."
        TEMPLATE_OPTION=" --template $LICENSE_UTILS_PATH/templates/Scopy/LICENSE"
        PARAMS_OPTION=" --params $LICENSE_UTILS_PATH/templates/Scopy/params.conf"
        $LICENSE_UTILS_PATH/batch_add_license_headers.sh $RESULTS_PATH/scan_output.txt $TEMPLATE_OPTION $PARAMS_OPTION
        ERROR_CODE=$?
        popd || exit 1 >/dev/null

        if [ $ERROR_CODE -ne 0 ]; then
            echo "### Error: Failed to add missing headers"
            exit 1
        else
            echo "### Success: Missing headers added"
        fi
    elif [ $EXIT_CODE -eq 0 ]; then
        echo "### All scanned files contain license headers ... no action required"
    else
        echo "### Erorr: Unknown error"
        exit 1
    fi
    echo "# Formatting files ..."
    "$REPOSTORY"/tools/format.sh >/dev/null 2>&1
    exit 0
}

main "$@"

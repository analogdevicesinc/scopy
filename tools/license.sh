#!/bin/bash

# set -x

CI_SCRIPT=${CI_SCRIPT:-"OFF"}

REPOSTORY=$(git rev-parse --show-toplevel)
PATH_OPTION=" --path $REPOSTORY"
OMIT_DIRS_OPTION=" --dirs .git*,build,.vscode,android,apple,ci,docs,tmp,tools,windows"
OMIT_FILES_OPTION=" --files .clang-format,.clangformatignore,.cmake-format,.gitignore,.gitmodules,*.md,LICENSE,*.png,*.rst,azure-pipelines.yml,requirements.txt,*.html,*.svg,*.icns,*.ico,*.qmodel,*.ui,*.json,*.qrc,*.ts,*.gif,*.theme,*.ttf,*.zip,*.csv,*.bin,*.xml,*.cmakein"

VERBOSE=""
CLEANUP="true"
# VERBOSE: Increasese processing time but provides detailed output. Use for manual inspection
# VERBOSE="--verbose"
# CLEANUP="" # true

LICENSE_UTILS_PATH=$REPOSTORY/tools/license-header
RESULTS_PATH=$LICENSE_UTILS_PATH/results

cleanup() {
    if [ "$CLEANUP" == "true" ] && [ -d "$RESULTS_PATH" ]; then
        rm -rf "$RESULTS_PATH"
    fi
}

main() {
    mkdir -p "$RESULTS_PATH"
    if [ -f "$RESULTS_PATH/scan.txt" ]; then
        rm "$RESULTS_PATH/scan.txt"
    fi
    touch "$RESULTS_PATH"/scan.txt

    echo "# Running license scanner utility ..."
    $LICENSE_UTILS_PATH/scan_missing_headers.sh \
        $PATH_OPTION \
        $OMIT_DIRS_OPTION \
        $OMIT_FILES_OPTION \
        $VERBOSE \
        >$RESULTS_PATH/scan.txt
    EXIT_CODE=$?
    echo "## Stored results under: $RESULTS_PATH/scan.txt"

    # CI should fail if missing headers are detected
    if [ "$CI_SCRIPT" == "ON" ]; then
        echo "## Report results:"
        cat "$RESULTS_PATH"/scan.txt
        exit $EXIT_CODE
    fi

    if [ $EXIT_CODE -ne 0 ] && [ "$VERBOSE" != "--verbose" ]; then
        pushd "$LICENSE_UTILS_PATH" || exit 1
        echo "### Warning: Missing headers found"

        TEMPLATE_OPTION=" --template $LICENSE_UTILS_PATH/templates/Scopy/LICENSE"
        PARAMS_OPTION=" --params $LICENSE_UTILS_PATH/templates/Scopy/params.conf"
        $LICENSE_UTILS_PATH/batch_add_license_headers.sh $RESULTS_PATH/scan.txt $TEMPLATE_OPTION $PARAMS_OPTION
        ERROR_CODE=$?
        popd || exit 1

        if [ $ERROR_CODE -ne 0 ]; then
            echo "### Error: Failed to add missing headers"
            exit 1
        else
            echo "### Success: Missing headers added"
        fi

        echo "# Formatting files ..."
        "$REPOSTORY"/tools/format.sh >/dev/null 2>&1
        exit 0
    else
        echo "### Success: All scanned files contain license headers"
    fi
}

trap cleanup EXIT
main "$@"

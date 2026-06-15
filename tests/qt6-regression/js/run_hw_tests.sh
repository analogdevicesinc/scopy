#!/bin/bash
# Qt6 Hardware Test Runner — ADALM-PLUTO
# Runs all hardware tests and saves results to log files.
#
# Usage: ./run_hw_tests.sh [path-to-scopy-binary]
# Example: ./run_hw_tests.sh ./build/scopy

SCOPY="${1:-./build/scopy}"
LOG_DIR="tests/qt6-regression/js/logs"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

mkdir -p "$LOG_DIR"

echo "================================================"
echo "Qt6 Hardware Test Runner"
echo "Scopy binary: $SCOPY"
echo "Log directory: $LOG_DIR"
echo "Timestamp: $TIMESTAMP"
echo "================================================"
echo ""

TESTS=(
    "qt6_hw_connection_test:tests/qt6-regression/js/qt6_hw_connection_test.js"
    "ad936xTests:js/testAutomations/ad936x/ad936xTests.js"
    "ad936xAdvancedTests:js/testAutomations/ad936x/ad936xAdvancedTests.js"
    "qt6_hw_regmap_test:tests/qt6-regression/js/qt6_hw_regmap_test.js"
    "qt6_hw_datalogger_test:tests/qt6-regression/js/qt6_hw_datalogger_test.js"
    "qt6_hw_iioexplorer_test:tests/qt6-regression/js/qt6_hw_iioexplorer_test.js"
)

TOTAL=0
PASSED=0
FAILED=0

for entry in "${TESTS[@]}"; do
    NAME="${entry%%:*}"
    SCRIPT="${entry##*:}"
    LOGFILE="$LOG_DIR/${NAME}_${TIMESTAMP}.log"
    TOTAL=$((TOTAL + 1))

    echo "--- [$TOTAL/${#TESTS[@]}] Running: $NAME ---"
    echo "    Script: $SCRIPT"
    echo "    Log:    $LOGFILE"

    "$SCOPY" --script="../$SCRIPT" > "$LOGFILE" 2>&1
    EXIT_CODE=$?

    # Check for failures in log
    FAIL_COUNT=$(grep -c "FAIL\|ERROR" "$LOGFILE" 2>/dev/null || echo "0")
    PASS_COUNT=$(grep -c "PASS" "$LOGFILE" 2>/dev/null || echo "0")

    if [ "$FAIL_COUNT" -gt 0 ]; then
        echo "    RESULT: FAILURES DETECTED ($FAIL_COUNT)"
        echo "    Failed tests:"
        grep -E "FAIL|ERROR" "$LOGFILE" | head -20
        FAILED=$((FAILED + 1))
    else
        echo "    RESULT: ALL PASSED ($PASS_COUNT)"
        PASSED=$((PASSED + 1))
    fi
    echo ""
done

echo "================================================"
echo "Summary: $PASSED/$TOTAL test suites passed, $FAILED failed"
echo "Logs saved in: $LOG_DIR/"
echo "================================================"

ls -la "$LOG_DIR/"*"$TIMESTAMP"* 2>/dev/null

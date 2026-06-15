#!/bin/bash
# Repeatedly connect to PLUTO until the PlotMarkerController crash occurs.
# Logs each attempt; stops when a crash is detected.
#
# Usage: from repo root: ./tests/qt6-regression/js/run_crash_repro.sh [max-attempts]
# Example: ./tests/qt6-regression/js/run_crash_repro.sh 50

MAX="${1:-50}"
REPO_ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
LOG_DIR="$REPO_ROOT/tests/qt6-regression/js/logs"
mkdir -p "$LOG_DIR"

cd "$REPO_ROOT/build" || { echo "ERROR: build/ directory not found"; exit 1; }

echo "Attempting to reproduce PlotMarkerController crash"
echo "Working dir: $(pwd)"
echo "Max attempts: $MAX"
echo ""

for i in $(seq 1 "$MAX"); do
    LOGFILE="$LOG_DIR/crash_repro_attempt_${i}.log"
    echo -n "Attempt $i/$MAX ... "

    ./scopy --script=../tests/qt6-regression/js/qt6_hw_crash_repro.js > "$LOGFILE" 2>&1
    EXIT_CODE=$?

    if grep -q "pure virtual\|terminate\|SIGSEGV\|SIGABRT" "$LOGFILE" 2>/dev/null; then
        echo "CRASHED! (exit code $EXIT_CODE)"
        echo ""
        echo "Crash log saved to: $LOGFILE"
        echo "--- Crash details ---"
        grep -A30 "pure virtual\|terminate\|SIGSEGV\|SIGABRT" "$LOGFILE" | head -40
        echo ""
        echo "Reproduced after $i attempts."
        exit 1
    elif grep -q "CLEAN_EXIT" "$LOGFILE" 2>/dev/null; then
        echo "OK (no crash)"
        rm -f "$LOGFILE"
    else
        echo "UNKNOWN (exit code $EXIT_CODE) — log kept"
    fi
done

echo ""
echo "No crash in $MAX attempts."
exit 0

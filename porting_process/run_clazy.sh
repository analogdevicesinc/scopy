#!/usr/bin/env bash
# Subtask 14: Clazy Qt6 Verification Run
# Run inside the scopy-qt6-dev Docker container.
# Prerequisites: clazy installed (apt-get install -y clazy)
# Usage: bash /home/runner/scopy/porting_process/run_clazy.sh

set -euo pipefail

SCOPY_ROOT="/home/runner/scopy"
BUILD_DIR="$SCOPY_ROOT/build-qt6"
OUT_DIR="$SCOPY_ROOT/porting_process"
REPORT="$OUT_DIR/clazy_qt6_report.txt"
FINDINGS="$OUT_DIR/clazy_qt6_findings.txt"
FILELIST="/tmp/clazy_filelist.txt"

CHECKS="qt6-deprecated-api-fixes,qt6-header-fixes,qt6-qhash-signature,qt6-fwd-fixes,missing-qobject-macro"

echo "=== Subtask 14: Clazy Qt6 Verification Run ==="
echo ""

# ── Sanity checks ──────────────────────────────────────────────────────────────
if ! command -v clazy-standalone &>/dev/null; then
    echo "ERROR: clazy-standalone not found. Run: apt-get install -y clazy"
    exit 1
fi

if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
    echo "ERROR: $BUILD_DIR/compile_commands.json not found."
    echo "       Build with cmake -B build-qt6 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON first."
    exit 1
fi

echo "clazy-standalone: $(clazy-standalone --version 2>&1 | head -1)"
echo "compile_commands.json: $(wc -l < "$BUILD_DIR/compile_commands.json") lines"
echo ""

# Note: --list-checks does not show "manual" level checks, which includes the Qt6
# porting checks. Absence from --list-checks does not mean the checks are unavailable.
# clazy-standalone will error explicitly if a check name is truly unknown.

# ── Step 1: Collect source files ───────────────────────────────────────────────
echo "=== Step 1: Collecting source files ==="
find "$SCOPY_ROOT" -name "*.cpp" \
    -not -path "*/build*" \
    -not -path "*/.git/*" \
    -not -path "*/packages/m2k/*" \
    -not -path "*/porting_debug/*" \
    -not -path "*/porting_process/*" \
    > "$FILELIST"

FILE_COUNT=$(wc -l < "$FILELIST")
echo "Found $FILE_COUNT .cpp files to scan (M2K excluded)"
echo ""

if [[ "$FILE_COUNT" -eq 0 ]]; then
    echo "ERROR: No source files found. Check SCOPY_ROOT path."
    exit 1
fi

# ── Step 2: Run clazy-standalone ──────────────────────────────────────────────
echo "=== Step 2: Running clazy-standalone ==="
echo "Checks: $CHECKS"
echo "Output: $REPORT"
echo "(This may take several minutes...)"
echo ""

# clazy-standalone exits non-zero if any warning is emitted; use || true
clazy-standalone \
    -p "$BUILD_DIR" \
    --checks="$CHECKS" \
    $(cat "$FILELIST") \
    2>&1 | tee "$REPORT" || true

echo ""
echo "=== Step 3: Filtering results ==="

# Extract unique warning lines for the 5 Qt6 checks.
# clazy formats warnings as -Wclazy-<checkname> (not [clazy-...]).
# Exclude warnings originating inside the Qt installation itself (/opt/Qt/).
grep -E "warning:.*-Wclazy-(qt6|missing-qobject)" "$REPORT" \
    | grep -v "^/opt/Qt/" \
    | sort -u \
    > "$FINDINGS" || true

FINDING_COUNT=$(wc -l < "$FINDINGS")

echo ""
echo "=== Results ==="
echo "Total lines in report : $(wc -l < "$REPORT")"
echo "Qt6-check warnings    : $FINDING_COUNT"
echo ""

if [[ "$FINDING_COUNT" -eq 0 ]]; then
    echo "SUCCESS: Zero Qt6 porting issues found. Migration verified clean."
else
    echo "FINDINGS ($FINDING_COUNT warnings):"
    echo "---"
    cat "$FINDINGS"
    echo "---"
    echo ""
    echo "Warning breakdown by check:"
    grep -oE "\[clazy-[a-z6-]+\]" "$FINDINGS" | sort | uniq -c | sort -rn || true
fi

echo ""
echo "Full report : $REPORT"
echo "Findings    : $FINDINGS"

# ── Write compact summary ──────────────────────────────────────────────────────
SUMMARY="$OUT_DIR/clazy_qt6_summary.txt"
{
    echo "=== Clazy Qt6 Scan Summary ==="
    echo "Date        : $(date)"
    echo "Files scanned : $FILE_COUNT"
    echo "Report lines  : $(wc -l < "$REPORT")"
    echo "Qt6 warnings  : $FINDING_COUNT"
    echo ""
    if [[ "$FINDING_COUNT" -eq 0 ]]; then
        echo "RESULT: CLEAN — zero Qt6 porting issues found."
    else
        echo "RESULT: FINDINGS"
        echo ""
        echo "Breakdown by check:"
        grep -oE "\[clazy-[a-z6-]+\]" "$FINDINGS" | sort | uniq -c | sort -rn || true
        echo ""
        echo "All warnings:"
        cat "$FINDINGS"
    fi
} > "$SUMMARY"

echo "Summary     : $SUMMARY  ← share this file"

# /verify-package — Validate package against CI checks

Run the same formatting and license checks that Scopy CI runs, scoped to a single package. This catches issues before pushing.

**Package to verify:** `$ARGUMENTS`

## Instructions

Run all 3 checks below against `scopy/packages/$ARGUMENTS/`. Report a clear pass/fail for each check with actionable fix instructions on failure.

### Check 1: C++ Formatting (clang-format)

Use the Glob tool to find all `.h`, `.hpp`, `.cpp`, `.cc` files in `scopy/packages/$ARGUMENTS/`, excluding:
- `build*/` directories
- Generated files: `*_export.h`, `*_config.h`, `moc_*.cpp`, `ui_*.h`

For each file, run:
```bash
clang-format --dry-run --Werror --style=file:scopy/.clang-format <file>
```

This mirrors the CI job `check-cpp-formatting` in `scopy/.github/workflows/clang-format.yml`.

On failure, show the files that need formatting and tell the user to run:
```
clang-format -i --style=file:scopy/.clang-format <file>
```

### Check 2: CMake Formatting (cmake-format)

Use the Glob tool to find all `CMakeLists.txt` and `*.cmake` files in `scopy/packages/$ARGUMENTS/`, excluding `build*/` directories.

For each file, run:
```bash
cmake-format --check --config-files scopy/.cmake-format <file>
```

On failure, show the files that need formatting and tell the user to run:
```
cmake-format -i --config-files scopy/.cmake-format <file>
```

### Check 3: License Headers (Scopy GPL)

Run the license scanner scoped to the package directory:

```bash
cd scopy && source tools/exclude_list.sh && \
tools/license-header/scan_missing_headers.sh \
  --path packages/$ARGUMENTS \
  --dirs "$(IFS=,; echo "${OMIT_DIRS_LIST[*]},${GITIGNORE_DIRS_LIST[*]}")" \
  --files "$(IFS=,; echo "${OMIT_FILES_LIST[*]},${GITINORE_FILES_LIST[*]}")"
```

On failure, show the files missing headers and tell the user to run:
```
cd scopy && ./tools/license.sh
```

### Report

After running all 3 checks, present a summary table:

```
## Package Verification: $ARGUMENTS

| Check              | Result |
|--------------------|--------|
| C++ Formatting     | PASS/FAIL |
| CMake Formatting   | PASS/FAIL |
| License Headers    | PASS/FAIL |
```

For any FAIL, list the specific files and the fix command above.

## Important

- Run each check independently — don't stop on first failure
- Show stderr/stdout from failed commands so the user can see the exact issues
- If a tool (clang-format, cmake-format) is not installed, report that clearly instead of failing silently

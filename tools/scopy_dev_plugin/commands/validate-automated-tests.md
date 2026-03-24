# /validate-automated-tests — Validate JS automated test scripts for a Scopy plugin

You are validating the JavaScript automated test scripts for the Scopy plugin: `$ARGUMENTS`

The `test-automation-quality-checks` skill rules (checks T1–T7) govern this analysis.

## Step 1: Discover files

Use the Glob tool to locate:
- Main test file: `js/testAutomations/$ARGUMENTS/*Tests.js` (exclude visual test files)
- Visual test file: `js/testAutomations/$ARGUMENTS/*VisualTests.js`
- Primary API header: `scopy/packages/$ARGUMENTS/plugins/*/include/**/*_api.h`
- RST test documentation: `scopy/packages/$ARGUMENTS/doc/*tests*.rst` or similar
- Test framework: `js/testAutomations/testFramework.js`

If no JS test files are found, report "No automated test files found for `$ARGUMENTS`" and stop.

**Cross-plugin header discovery:** After reading the JS test files, scan them for API object
references using the pattern `<objectName>.<method>()`. Collect every distinct object name
that is not `TestFramework`, `scopy`, or a built-in (e.g. `msleep`, `assertEqual`).
For each such name, use the Glob tool to search for a matching `*_api.h` across all packages:
`scopy/packages/*/plugins/*/include/**/*_api.h`. Read any headers found so that [T2] can
validate cross-plugin method calls against the correct plugin's header.

Read **all** discovered files (primary + cross-plugin) before starting analysis.

## Step 2: Run checks T1–T7

Apply every rule from the `test-automation-quality-checks` skill:

### CRITICAL

**[T1] UID Matching**
- Extract every UID from `TestFramework.runTest("<UID>", ...)` calls in the JS files
- Extract every test UID from the RST documentation (Category A and B)
- Flag any JS UID that does not exist in the RST doc
- Flag any RST Category A/B UID that has no corresponding JS test

**[T2] API Method Validity**
- List every `<apiObject>.<method>()` call in the JS files
- For each call, determine which plugin the API object belongs to:
  - If the object name matches the primary plugin's `setObjectName()` value → validate against the primary plugin's `*_api.h`
  - Otherwise → find the cross-plugin header discovered in Step 1 whose filename or `setObjectName()` matches the object name → validate against that header
- Flag any call to a method not found in the correct plugin's header
- Flag any API object whose header could not be located

**[T3] Sleep After Setters**
- Identify every setter call (any method matching `set*()` or writing a value)
- Check that the very next non-empty line is `msleep(500)`
- Flag any setter not followed immediately by `msleep(500)`

**[T4] State Restoration**
- Identify every test that modifies state (calls a setter)
- Check that it saves the original value before the modification
- Check that it restores the original value in normal completion, early return, and catch block
- Flag any test that modifies state without restoring it

### WARNING

**[T5] Visual Test Pattern**
- Check that the visual test file uses `TestFramework.supervisedCheck()` — not `msleep(VISUAL_DELAY)`
- Flag any use of `VISUAL_DELAY` inside the visual test file

**[T6] File Termination**
- Check that both files end with `disconnectFromDevice()` → `printSummary()` → `scopy.exit()` in that order
- Flag any file that deviates from this sequence

### INFO

**[T7] Coverage**
- List all RST Category A/B UIDs and whether each has a JS implementation
- Report the coverage ratio

## Step 3: Generate report

```
## Automated Test Validation Report: $ARGUMENTS

### Summary
| Check | Severity | Result |
|-------|----------|--------|
| [T1] UID Matching | CRITICAL | PASS/FAIL |
| [T2] API Method Validity | CRITICAL | PASS/FAIL |
| [T3] Sleep After Setters | CRITICAL | PASS/FAIL |
| [T4] State Restoration | CRITICAL | PASS/FAIL |
| [T5] Visual Test Pattern | WARNING | PASS/WARN |
| [T6] File Termination | WARNING | PASS/WARN |
| [T7] Coverage | INFO | X/Y UIDs covered |

### Critical Issues
**[T2] Invented API method**
`js/testAutomations/myplugin/myTests.js:42` — `mypluginApi.setFoo()` does not exist in `myplugin_api.h`.
> **Fix:** Use the correct method name from the API header, or add the method to the API class.

### Warnings
...

### Info
**[T7] Coverage**
- RST Category A: 5 UIDs — 5 implemented (100%)
- RST Category B: 3 UIDs — 2 implemented (67%) — missing: `MY_PLUGIN_0003`

### Verdict
[PASS/FAIL] — [one sentence summary]
```

PASS = zero critical issues. FAIL = one or more critical issues.

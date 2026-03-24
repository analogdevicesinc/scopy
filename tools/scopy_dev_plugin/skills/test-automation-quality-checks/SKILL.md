---
name: test-automation-quality-checks
description: JS automated test validation rules for Scopy plugins. Auto-loads when reviewing or writing test automation scripts in `js/testAutomations/`.
---

# JS Test Automation Quality Check Rules

Apply these 7 validation categories when reviewing or generating JS automated test scripts for Scopy plugins.

---

## CRITICAL

### [T1] UID Matching

- Every `TestFramework.runTest("<UID>", ...)` UID must exist **exactly** in the RST test documentation
- No invented UIDs that are absent from the RST doc
- No Category A/B RST UIDs that are absent from the JS test files

### [T2] API Method Validity

- Every `<apiObject>.<method>()` call must exist as a `Q_INVOKABLE` in the correct plugin's `*_api.h`
- No invented or assumed method names — verify against the actual header
- For cross-plugin calls (where the API object belongs to a different plugin than the one under
  test): identify the API object name → match it to the plugin whose `initApi()` calls
  `setObjectName("<name>")` with that name → validate the method against THAT plugin's `*_api.h`
- Do not validate cross-plugin methods against the primary plugin's header

### [T3] Sleep After Setters

- Every setter call must be followed **immediately** by `msleep(500)` on the next non-empty line
- This applies to setter calls on **any** plugin API object, including cross-plugin setters
  invoked after `switchToTool()`
- No setter call may be left without a subsequent sleep

### [T4] State Restoration

- Every test that modifies plugin state must save the original value before changing it
- The original value must be restored in **all** exit paths: normal completion, early return, and catch block

---

## WARNING

### [T5] Visual Test Pattern

- Visual test files must use `TestFramework.supervisedCheck()` — never use `msleep(VISUAL_DELAY)` in visual test files
- The visual delay constant (`VISUAL_DELAY`) must not appear in visual test files

### [T6] File Termination

- Both the main test file and the visual test file must end with this sequence in order:
  1. `disconnectFromDevice()`
  2. `printSummary()`
  3. `scopy.exit()`

---

## INFO

### [T7] Coverage

- Compare RST Category A and Category B UIDs against JS test names
- Flag any Category A/B UIDs that have no corresponding JS test implementation

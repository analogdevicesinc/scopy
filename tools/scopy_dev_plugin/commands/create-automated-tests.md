# /create-automated-tests — Create JS automated test scripts

You are creating JavaScript automated test scripts for a Scopy plugin, mapping RST manual test cases to API calls.

**Plugin:** `$ARGUMENTS`

## Step 0: Load context

Use the Read tool to check if a port state file exists:
- Path: `tasks/$ARGUMENTS-port-state.md`
- If the file does not exist, note "No state file — will discover from source files directly." and continue.

## Prerequisites check

Before writing tests, verify these inputs exist:
1. **Plugin API class** — use the Glob tool to search for `*_api.h` in `scopy/packages/$ARGUMENTS/`
2. **Test RST docs** — use the Read tool on `scopy/docs/tests/plugins/$ARGUMENTS/$ARGUMENTS_tests.rst`

If the API class does not exist, stop and tell the user to run `/create-api $ARGUMENTS` first.
If the test RST does not exist, stop and tell the user to run `/create-test-docs $ARGUMENTS` first.

## Step 1: Discovery

Read these specific files:

1. **Plugin API header** (all `Q_INVOKABLE` methods):
   - Use Glob: `scopy/packages/$ARGUMENTS/plugins/*/include/*/$ARGUMENTS*_api.h`

2. **Test RST documentation** (all UIDs and steps):
   - `scopy/docs/tests/plugins/$ARGUMENTS/$ARGUMENTS_tests.rst`
   - `scopy/docs/tests/plugins/$ARGUMENTS/$ARGUMENTS_advanced_tests.rst` (if exists)

3. **Existing test files** (check for update mode):
   - Use Glob: `scopy/js/testAutomations/$ARGUMENTS/`
   - If test files already exist, switch to **Update Mode** (see below).

4. **Test framework API**:
   - `scopy/js/testAutomations/common/testFramework.js`

5. **Cross-plugin APIs** (if RST steps reference other plugins):
   - Read headers for any plugin mentioned in test steps

## Update Mode (existing tests)

When JS test files already exist in `scopy/js/testAutomations/$ARGUMENTS/`, do NOT create new files from scratch. Instead:

1. **Read all existing test files** completely.
2. **Run all T1–T7 checks** from the `test-automation-quality-checks` skill against the existing code (same as `/validate-automated-tests`).
3. **Present a fix plan** listing:
   - Critical issues found (T1–T7 failures) with file:line references
   - Missing test coverage (RST UIDs without JS tests)
4. **AUTO-APPROVE and proceed** — apply all fixes to the existing test files:
   - [T3] Add `msleep(500)` after every setter call that's missing it
   - [T4] Add state save/restore in catch blocks and early return paths
   - [T5] Replace `VISUAL_DELAY` / `msleep(VISUAL_DELAY)` with `TestFramework.supervisedCheck("description")`
   - [T6] Fix file termination sequence if incorrect
   - Add missing tests for uncovered RST UIDs (Category A/B only)
5. Continue to **Step 4: Validate** below to confirm all checks pass after fixes.

Do NOT regenerate tests from scratch — fix the existing code in place, preserving existing test logic.

---

## Step 2: Classify — WAIT FOR APPROVAL

After reading all source material, present a classification report:

### Category A: Fully Automatable
All steps AND expected results can be verified via API (write-readback, getTools, switchToTool).

### Category B: Supervised (Visual Check Required)
Steps automatable but expected result requires human visual observation (`supervisedCheck()`).

### Category C: Not Automatable
No API exists for required step, requires OS dialogs, external hardware, or Scopy restart.

**Wait for user approval before writing any JavaScript.**

## Step 3: Implement

**File locations:**
- `scopy/js/testAutomations/$ARGUMENTS/<plugin>DocTests.js` — Category A
- `scopy/js/testAutomations/$ARGUMENTS/<plugin>VisualTests.js` — Category B

**Critical rules (non-negotiable):**
1. Every setter is followed by `msleep(500)`
2. Every test saves original value before modifying, and restores it in ALL code paths
3. Visual tests use `TestFramework.supervisedCheck("description")` — never `msleep(VISUAL_DELAY)`
4. Test names in `TestFramework.runTest()` must be the exact UID from the RST doc
5. Never invent API methods — only use what's in the `*_api.h` header
6. Files end with `TestFramework.disconnectFromDevice()`, `TestFramework.printSummary()`, `scopy.exit()`
7. Category C tests get a Missing API Report comment block at the top of DocTests.js

## Step 4: Validate

- [ ] Every test name matches the RST UID exactly
- [ ] Every setter has `msleep(500)` after it
- [ ] Original values saved and restored in all code paths (success, failure, catch)
- [ ] No invented API methods — only methods from `*_api.h`
- [ ] Visual tests use `supervisedCheck()` not `msleep(VISUAL_DELAY)`
- [ ] Files end with disconnect, printSummary, exit sequence
- [ ] Category C tests documented in Missing API Report

## Step 5: Update state file (if it exists)

```markdown
## Status
- Phase: AUTO_TESTS_COMPLETE
```

## Rules

- Do NOT modify any C++ source code
- Do NOT use `msleep(VISUAL_DELAY)` — use `TestFramework.supervisedCheck()` for visual checks
- Do NOT invent API methods that don't exist in the `*_api.h` header
- Getter return values are always strings — compare with `===` against string values
- Every test must restore original hardware state

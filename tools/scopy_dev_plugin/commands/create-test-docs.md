# /create-test-docs — Generate RST test case documentation

You are creating Sphinx RST test case documentation for a Scopy plugin, following the established test documentation format.

**Plugin:** `$ARGUMENTS`

## Step 0: Load context

Use the Read tool to check if a port state file exists:
- Path: `tasks/$ARGUMENTS-port-state.md`
- If the file does not exist, note "No state file — will discover from source files directly." and continue.

Also use the Glob tool to check if plugin docs already exist:
- Pattern: `scopy/docs/plugins/$ARGUMENTS/`
- If nothing is found, note "No plugin docs yet — run /create-docs first for richer test coverage."

## Step 1: Discovery

**If state file exists**, extract:
- Section list (all tabs and sub-tabs) for deriving test scenarios
- Attribute inventory per section for read/write test coverage
- Device name and hardware requirements

**If no state file**, discover from:
- Plugin docs (if created): `scopy/docs/plugins/$ARGUMENTS/$ARGUMENTS.rst` — section/attribute list
- Plugin source files: tool class headers — section and attribute names
- `manifest.json.cmakein` — device name and display name

Read the reference prompt for UID conventions and implementation patterns:
`scopy/tools/prompts/scopy_test_docs_agent.md`

Study these reference test RST files before writing:
- `scopy/docs/tests/plugins/ad936x/ad936x_tests.rst` — full test suite format
- `scopy/docs/tests/plugins/adrv9002/adrv9002_tests.rst` — multi-section format
- `scopy/docs/tests/plugins/ad936x/index.rst` — index structure
- `scopy/docs/tests/plugins/index.rst` — top-level toctree to update

## Step 2: Plan — WAIT FOR APPROVAL

Present a test plan:

1. **File list** to create
2. **Test inventory** — one row per planned test:
   | # | UID | Title | RBP | Section | Type |
3. **UID naming scheme** — confirm the prefix (e.g., `TST.AD9371`)
4. **Setup environments** referenced in prerequisites
5. **toctree update** in `scopy/docs/tests/plugins/index.rst`

**RBP Assignment Rules:**
- **P0** — Plugin loads, basic plugin detection
- **P1** — Core read/write functionality for primary attributes
- **P2** — Advanced tab attributes, edge case settings
- **P3** — Diagnostics, display tests, cosmetic checks

**Wait for user approval before writing any RST.**

## Step 3: Implement

Follow the test RST format exactly as in reference files.

### Test entry format
```rst
Test N: <Title>
~~~~~~~~~~~~~~~~~~

**UID:** TST.<PLUGIN>.<TEST_NAME>

**RBP:** P0

**Description:** <One sentence describing what this test verifies.>

**OS:** ANY

**Preconditions:**
    - <Hardware requirement>

**Steps:**
    1. <Action step>
        - **Expected result:** <What should happen>
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..
```

### Standard test sequence for attribute read/write tests
Each section should have at minimum:
1. A "section visible" P1 test
2. One write-readback test per key attribute
3. A "refresh" test if the plugin has a refresh button

## Step 4: Validate

- [ ] Every UID is unique — no duplicate UIDs
- [ ] UIDs follow `TST.<PLUGIN>.<TEST_NAME>` format with UPPER_SNAKE_CASE
- [ ] Every section from the plugin has at least one test
- [ ] P0 "Plugin Loads" test always present as the first test
- [ ] Heading underlines are the correct length
- [ ] toctree in index.rst matches actual file names
- [ ] `scopy/docs/tests/plugins/index.rst` has the new plugin added
- [ ] `**Actual result:**` lines are empty

## Step 5: Update state file (if it exists)

```markdown
## Status
- Phase: TEST_DOCS_COMPLETE
```

## Rules

- Match the exact format of `scopy/docs/tests/plugins/ad936x/ad936x_tests.rst`
- Do NOT fill in "Actual result:" fields — leave them empty
- Do NOT fill in "Tested OS:" or "Comments:" fields — leave them empty
- Do NOT fill in "Result:" — it stays as `PASS/FAIL`
- Every test must have a unique UID
- Keep test steps short and action-focused
- Expected results must be verifiable (not vague like "works correctly")

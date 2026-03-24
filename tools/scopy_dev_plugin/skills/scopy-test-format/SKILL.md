---
name: scopy-test-format
description: Test case conventions for Scopy plugins including UID format, RBP priorities, RST test entry templates, and standard test sequences. Auto-loads when writing test case documentation or automated test scripts.
---

# Scopy Test Case Conventions

## UID Convention

Format: **`TST.<PLUGIN_UPPER>.<TEST_NAME_UPPER_SNAKE>`**

Examples:
- `TST.AD9371.PLUGIN_LOADS`
- `TST.AD9371.RX_LO_FREQUENCY_READBACK`
- `TST.AD9371.ADVANCED_PANEL_VISIBLE`

Rules:
- `<PLUGIN_UPPER>` = plugin folder name uppercased (e.g., `ad9371` -> `AD9371`)
- `<TEST_NAME>` = short descriptive name in `UPPER_SNAKE_CASE`
- UIDs must be unique across all existing test files in `scopy/docs/tests/plugins/`

## RBP Priority Levels

| Priority | When to use |
|----------|-------------|
| **P0** | Plugin loads test — must be first test, must pass for anything else to matter |
| **P1** | Core read/write for primary attributes in each main section |
| **P2** | Advanced tab attributes, optional features, secondary settings |
| **P3** | Diagnostics, display/cosmetic tests, edge cases, multi-device scenarios |

Every plugin must have exactly one P0 test: `Plugin Loads`.

## Standard Test Sequence Per Section

For each tab/section in the plugin, write at minimum:

1. **Section visible** (P1) — open plugin, switch to section, verify it is displayed
2. **Key attribute write-readback** (P1) — change a value, confirm the hardware reflects it
3. **Refresh** (P1) — click the refresh button and confirm displayed values update (only if plugin has a refresh button)

## RST Test Entry Template

```rst
Test N: <Title>
~~~~~~~~~~~~~~~~~~

**UID:** TST.<PLUGIN>.<TEST_NAME>

**RBP:** P0

**Description:** <One sentence describing what this test verifies.>

**OS:** ANY

**Preconditions:**
    - <Hardware or software requirement>

**Steps:**
    1. <Action step>
        - **Expected result:** <Specific verifiable outcome>
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

## Prerequisites Block (top of test file)

```rst
.. _<plugin>_tests:

<Device Name> Test Suite
================================================================================

**Prerequisites:**
    - Scopy vX.Y or later with <Plugin Name> plugin installed
    - <Device name> device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences
```

## Content Rules

- **Do NOT fill in** `Actual result:`, `Tested OS:`, `Comments:`, or `Result:` fields
- `Result:` always stays as the literal text `PASS/FAIL`
- Test steps must be action-focused (1-2 sentences each)
- Expected results must be verifiable — not vague phrases like "works correctly"
- Heading `~~` underlines must be at least as long as the heading text

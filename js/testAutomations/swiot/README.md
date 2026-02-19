# SWIOT Plugin - Test Automation

## Overview

This directory contains automated test scripts for the SWIOT plugin (AD-SWIOT1L-SL).
The tests are based on the manual test documentation from
`docs/tests/plugins/swiot1l/swiot1l_tests.rst`.

Tests are split into two scripts:
- **`swiotTests.js`** - Fully automated tests (no human supervision needed)
- **`swiotSupervisedTests.js`** - Semi-automated tests (require visual verification of plot signals)

## Hardware Setup

- Connect to AD-SWIOT1L-SL at `ip:169.254.97.40`
- Loopback cables:
  - CH_1 to CH_2 (signal + GND)
  - CH_3 to CH_4 (signal + GND)
- Runtime configuration:
  - Channel 1: ad74413r / voltage_out
  - Channel 2: ad74413r / voltage_in
  - Channel 3: max14906 / input
  - Channel 4: max14906 / output

## Running the Tests

```bash
cd scopy/build/

# Automated tests (no supervision needed)
./scopy --script ../js/testAutomations/swiot/swiotTests.js

# Supervised tests (require visual verification)
./scopy --script ../js/testAutomations/swiot/swiotSupervisedTests.js
```

## Test Classification

### Fully Automated (`swiotTests.js`)

These tests run without human intervention. All assertions are verified programmatically.

| Test | UID | Description |
|------|-----|-------------|
| 1 | TST.SWIOT.COMPAT | Plugin compatibility - verifies connection and 4 config channels |
| 2 | TST.SWIOT.MODES | Mode switching between Config and Runtime |
| 3 | TST.SWIOT.CONFIG | Config mode - channel configuration and Apply |
| 4 | TST.AD74413R.PLOT | Plot labels, timespan, single capture, measurements |
| 7 | TST.AD74413R.SAMPLING | Sampling frequency and computed sample rate |
| 9 | TST.MAX14906.PLOT | MAX14906 timespan and X axis verification |
| 10 | TST.MAX14906.CHANNEL | MAX14906 output on/off and channel operations |
| 12 | TST.FAULTS.RUN_SINGLE | Faults single capture and bit verification |
| 13 | TST.FAULTS.CLEAR_RESET | Faults clear selection and reset stored |
| 14 | TST.FAULTS.RUN_CONTINUOUS | Faults continuous capture and reset while running |

### Supervised (`swiotSupervisedTests.js`)

These tests automate the setup and attribute writes but require a human to verify
plot signals and measurement values on screen. Lines prefixed with `SUPERVISE:` in
the console output indicate what to visually check.

| Test | UID | Description | What to supervise |
|------|-----|-------------|-------------------|
| 5 | TST.AD74413R.CHANNEL | Channel operations with RAW output writes | Verify voltage_in 2 plot signal matches expected voltage (~10V, ~5.5V, 0V) after RAW changes |
| 6 | TST.AD74413R.DIAG | Diagnostic channels with diag_function | Verify diagnostic 5 and voltage_in 2 track the same voltage on the plot (~10V, ~5.5V, ~2.7V) |

### Manual Only (not scriptable)

These tests require UI interaction that cannot be driven through the scripting API.

| Test | UID | Description | Reason |
|------|-----|-------------|--------|
| 8 | TST.AD74413R.TUTORIAL | Tutorial start, follow, and exit | Requires clicking tutorial overlay buttons |
| 11 | TST.MAX14906.TUTORIAL | Tutorial start, follow, and exit | Requires clicking tutorial overlay buttons |
| 15 | TST.FAULTS.TUTORIAL | Tutorial start, follow, and exit | Requires clicking tutorial overlay buttons |
| 16 | TST.SWIOT.EXTERNAL_SUPPLY | External power supply detection | Requires physical power supply switch toggle |

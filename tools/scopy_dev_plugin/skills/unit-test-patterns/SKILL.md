---
name: unit-test-patterns
description: Code patterns and examples for Scopy IIOWidget unit tests. Covers standard helpers, data-driven testing, and complex multi-step scenarios. Auto-loads when creating or reviewing `*_Unit_test.js` files.
---

# Unit Test Patterns for Scopy IIOWidget Tests

This skill provides code patterns for generating and reviewing JS unit tests that validate IIOWidget attributes in Scopy plugins. All examples are from the AD9371 reference implementation.

## Standard Helper Functions

These helpers test individual widget attributes. Every unit test file must define and use them.

| Pattern | File | Widget Type | Description |
|---------|------|-------------|-------------|
| [testRange](test-range.md) | `test-range.md` | Range/Spinbox | Write min/max/mid, verify readback with tolerance |
| [testCombo](test-combo.md) | `test-combo.md` | Combo/Dropdown | Iterate valid options, verify exact match |
| [testCheckbox](test-checkbox.md) | `test-checkbox.md` | Checkbox/Toggle | Toggle 0/1, verify readback |
| [testReadOnly](test-readonly.md) | `test-readonly.md` | Read-Only | Verify value is non-empty |
| [testConversion](test-conversion.md) | `test-conversion.md` | API + Widget | Validate API scaling vs raw widget value |
| [testBadValueRange](test-bad-value-range.md) | `test-bad-value-range.md` | Range boundary | Verify above-max/below-min clamping |
| [testBadValueCombo](test-bad-value-combo.md) | `test-bad-value-combo.md` | Combo boundary | Verify invalid option rejection |
| [testCalibrationFlag](test-calibration-flag.md) | `test-calibration-flag.md` | Calibration toggle | Test enable/disable via API getter/setter |
| [runDataDrivenTests](data-driven-tests.md) | `data-driven-tests.md` | Batch dispatch | Run array of test descriptors through helpers |

## Complex Test Patterns

Multi-step scenarios that test functionality beyond single-widget read/write. Each pattern includes: when to use, API signature trigger, and complete code example.

| Pattern | File | Trigger |
|---------|------|---------|
| [Calibration Flow](complex-calibration-flow.md) | `complex-calibration-flow.md` | `calibrate()` exists in API |
| [Profile Loading](complex-profile-loading.md) | `complex-profile-loading.md` | `loadProfile()` exists |
| [Gain Mode Interaction](complex-gain-mode-interaction.md) | `complex-gain-mode-interaction.md` | `getXxxGainControlMode()` + `setXxxHardwareGain()` |
| [State Transitions](complex-state-transitions.md) | `complex-state-transitions.md` | `getEnsmMode()`/`setEnsmMode()` |
| [DPD Operations](complex-dpd-operations.md) | `complex-dpd-operations.md` | `dpdReset()` + `getDpdStatus()` |
| [Channel Independence](complex-channel-independence.md) | `complex-channel-independence.md` | Setter with `int channel`, count > 1 |
| [Phase Rotation](complex-phase-rotation.md) | `complex-phase-rotation.md` | `getPhaseRotation()`/`setPhaseRotation()` |
| [Frequency Tuning](complex-frequency-tuning.md) | `complex-frequency-tuning.md` | Getter/setter with Hz-to-MHz conversion |
| [UDC LO Splitting](complex-udc-lo-splitting.md) | `complex-udc-lo-splitting.md` | `hasUdc()`/`getUdcEnabled()` |
| [Refresh Cycle](complex-refresh-cycle.md) | `complex-refresh-cycle.md` | `refresh()` exists |

## File Structure

| Reference | File |
|-----------|------|
| [File Structure & Boilerplate](file-structure.md) | `file-structure.md` |

## Key Rules

1. **Every setter must be followed by `msleep(500)`** — no exceptions
2. **Every test must save and restore original values** — in all exit paths
3. **Use standard helpers** — do not write ad-hoc read/write/compare for standard widget types
4. **Use `runDataDrivenTests()`** — for sections with 3+ widgets of the same type
5. **UID format:** `UNIT.<SECTION>.<ATTRIBUTE_NAME>` — uppercase, dots as separators
6. **Widget key format:** `<device-name>/<channel>/<attribute>` or `<device-name>/<attribute>`

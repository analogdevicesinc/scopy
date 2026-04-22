# Complex: DPD Operations

## Description

Tests Digital Pre-Distortion (DPD) reset and status verification. Resets DPD, checks status string is valid, and verifies track count is readable. Typically AD9375-only.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains `dpdReset()` and `getDpdStatus()` methods.

## Skip Guard

```javascript
if (!isAd9375) {
    printToConsole("  SKIP: Not AD9375");
    return "SKIP";
}
```

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C6: DPD Reset + Status Check (AD9375 only)
TestFramework.runTest("UNIT.DPD.RESET_AND_STATUS_CH0", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }

        var statusBefore = ad9371.getDpdStatus(0);
        printToConsole("  DPD status before reset: " + statusBefore);

        ad9371.dpdReset(0);
        msleep(1000);

        var statusAfter = ad9371.getDpdStatus(0);
        printToConsole("  DPD status after reset: " + statusAfter);

        // Verify status is a valid human-readable string
        if (!statusAfter || statusAfter === "") {
            printToConsole("  FAIL: status empty after reset");
            return false;
        }
        if (statusAfter.indexOf("No Error") === -1 && statusAfter.indexOf("Error:") === -1) {
            printToConsole("  FAIL: status not a valid string: " + statusAfter);
            return false;
        }

        // Verify track count is readable
        var trackCount = ad9371.getDpdTrackCount(0);
        printToConsole("  DPD track count: " + trackCount);
        if (trackCount === null || trackCount === "") {
            printToConsole("  FAIL: track count unreadable after reset");
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Known Status Strings Pattern

For validating status strings, use a known-list pattern:

```javascript
var KNOWN_DPD_STATUS = [
    "No Error", "Error: ORx disabled", "Error: Tx disabled",
    "Error: DPD initialization not run", "Error: Path delay not setup",
    "Error: ORx signal too low", "Error: ORx signal saturated",
    "Error: Tx signal too low", "Error: Tx signal saturated",
    "Error: Model error high", "Error: AM AM outliers",
    "Error: Invalid Tx profile", "Error: ORx QEC Disabled"
];

function isKnownStatus(val, knownList) {
    for (var i = 0; i < knownList.length; i++) {
        if (val === knownList[i]) return true;
    }
    if (val.indexOf("Error: Unknown status") === 0) return true;
    return false;
}
```

## Key Requirements

- Skip guard for variant-specific features (e.g., `isAd9375`)
- Longer sleep after reset (1000ms)
- Validate status is a recognized string, not just non-empty
- Accept "Unknown status" patterns for undocumented firmware codes
- Same pattern applies to CLGC and VSWR subsystems

# Complex: Refresh Cycle

## Description

Tests the `refresh()` method by reading representative values before and after refresh, verifying they remain valid and consistent.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains a `refresh()` method.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C10: Refresh Cycle
TestFramework.runTest("UNIT.UTIL.REFRESH_ALL", function() {
    try {
        // Read representative values before refresh
        var ensmBefore = ad9371.getEnsmMode();
        var rxGainBefore = ad9371.getRxHardwareGain(0);
        printToConsole("  Before refresh: ensm=" + ensmBefore + " rxGain=" + rxGainBefore);

        // Trigger refresh
        ad9371.refresh();
        msleep(2000);

        // Read again after refresh
        var ensmAfter = ad9371.getEnsmMode();
        var rxGainAfter = ad9371.getRxHardwareGain(0);
        printToConsole("  After refresh: ensm=" + ensmAfter + " rxGain=" + rxGainAfter);

        // Values should be non-empty and consistent
        if (!ensmAfter || ensmAfter === "") {
            printToConsole("  FAIL: ENSM mode empty after refresh");
            return false;
        }
        if (!rxGainAfter || rxGainAfter === "") {
            printToConsole("  FAIL: RX gain empty after refresh");
            return false;
        }

        printToConsole("  PASS: refresh completed, values consistent");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- No state modification needed — this is a read-verify test
- Longer sleep after `refresh()` (2000ms) to allow all widgets to update
- Pick 2-3 representative values from different sections
- Verify values are non-empty, not necessarily identical to before

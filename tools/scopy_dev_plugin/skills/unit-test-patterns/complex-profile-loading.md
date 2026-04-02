# Complex: Profile Loading

## Description

Tests loading a device profile and verifying the device state is valid afterwards. Reads representative values before and after profile load.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains `loadProfile(QString)` and `getDefaultProfilePath()` methods.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C2: Profile Loading
TestFramework.runTest("UNIT.PROFILE.LOAD_AND_VERIFY", function() {
    try {
        var profilePath = ad9371.getDefaultProfilePath();
        if (!profilePath || profilePath === "") {
            printToConsole("  SKIP: No profile file available");
            return "SKIP";
        }
        printToConsole("  Profile path: " + profilePath);

        // Read current state
        var rxBwBefore = ad9371.getRxRfBandwidth();
        var txBwBefore = ad9371.getTxRfBandwidth();
        printToConsole("  Before: RX BW=" + rxBwBefore + " TX BW=" + txBwBefore);

        // Load profile
        ad9371.loadProfile(profilePath);
        msleep(5000);

        // Refresh widgets
        ad9371.refresh();
        msleep(2000);

        // Read after - verify sampling frequency is readable
        var rxSfAfter = ad9371.getRxSamplingFrequency();
        var txSfAfter = ad9371.getTxSamplingFrequency();
        printToConsole("  After: RX SF=" + rxSfAfter + " TX SF=" + txSfAfter);

        // Verify at least sampling frequency is readable after profile load
        if (!rxSfAfter || rxSfAfter === "" || rxSfAfter === "0.000000") {
            printToConsole("  FAIL: RX sampling frequency invalid after profile load");
            return false;
        }

        printToConsole("  PASS: profile loaded successfully");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Use `return "SKIP"` when no profile file is available
- Long sleep after `loadProfile()` (5000ms) — hardware reconfiguration takes time
- Call `refresh()` after profile load to update widget values
- Verify representative values are readable (not necessarily unchanged)

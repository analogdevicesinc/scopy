# Complex: Gain Mode Interaction

## Description

Tests the interaction between gain control mode (manual/automatic/hybrid) and hardware gain. Verifies that gain is writable in manual mode and readable in AGC mode.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains both `getXxxGainControlMode()`/`setXxxGainControlMode()` and `getXxxHardwareGain()`/`setXxxHardwareGain()` methods.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C4: Gain Mode Interaction
TestFramework.runTest("UNIT.RX.GAIN_MODE_INTERACTION", function() {
    try {
        var origMode = ad9371.getRxGainControlMode();
        var origGain = ad9371.getRxHardwareGain(0);
        printToConsole("  Original mode: " + origMode + ", gain: " + origGain);

        // Set to manual and set a specific gain
        ad9371.setRxGainControlMode("manual");
        msleep(1000);
        ad9371.setRxHardwareGain(0, "20");
        msleep(1000);
        var manualGain = ad9371.getRxHardwareGain(0);
        printToConsole("  Manual mode gain set to 20, read: " + manualGain);
        if (Math.abs(parseFloat(manualGain) - 20.0) > 1.0) {
            printToConsole("  FAIL: gain should be ~20 in manual mode");
            ad9371.setRxGainControlMode(origMode);
            msleep(500);
            return false;
        }

        // Switch to automatic (AGC)
        ad9371.setRxGainControlMode("automatic");
        msleep(1000);
        var agcGain = ad9371.getRxHardwareGain(0);
        printToConsole("  AGC mode gain: " + agcGain);
        if (agcGain === null || agcGain === "") {
            printToConsole("  FAIL: gain unreadable in AGC mode");
            ad9371.setRxGainControlMode(origMode);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setRxGainControlMode(origMode);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Use longer sleep (1000ms) after mode changes — hardware needs settling time
- Verify gain is controllable in manual mode AND readable in AGC mode
- Restore original mode in all exit paths
- Tolerance for gain comparison is wider (1.0 dB) due to AGC adjustments

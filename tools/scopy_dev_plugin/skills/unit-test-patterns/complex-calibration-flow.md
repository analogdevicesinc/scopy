# Complex: Calibration Flow

## Description

Tests the full calibration trigger flow: enable calibration flags, trigger calibration, verify flags remain set, restore originals.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains a `calibrate()` method along with `getCalibrateXxxEn()`/`setCalibrateXxxEn()` getter/setter pairs.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C1: Full Calibration Flow
TestFramework.runTest("UNIT.CAL.FULL_CALIBRATION_FLOW", function() {
    try {
        // Save originals
        var origRxQec = ad9371.getCalibrateRxQecEn();
        var origTxQec = ad9371.getCalibrateTxQecEn();
        var origTxLol = ad9371.getCalibrateTxLolEn();
        var origTxLolExt = ad9371.getCalibrateTxLolExtEn();

        // Enable all calibration flags
        ad9371.setCalibrateRxQecEn("1");
        ad9371.setCalibrateTxQecEn("1");
        ad9371.setCalibrateTxLolEn("1");
        ad9371.setCalibrateTxLolExtEn("1");
        msleep(500);

        // Trigger calibration
        ad9371.calibrate();
        msleep(2000);

        // Verify flags still set
        var rxQec = ad9371.getCalibrateRxQecEn();
        var txQec = ad9371.getCalibrateTxQecEn();
        printToConsole("  After calibrate: rx_qec=" + rxQec + " tx_qec=" + txQec);

        // Restore
        ad9371.setCalibrateRxQecEn(origRxQec);
        ad9371.setCalibrateTxQecEn(origTxQec);
        ad9371.setCalibrateTxLolEn(origTxLol);
        ad9371.setCalibrateTxLolExtEn(origTxLolExt);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Save ALL calibration flags before modifying any
- Use longer sleep after `calibrate()` (2000ms) — hardware needs time
- Restore all flags in all exit paths
- This test validates the calibration mechanism, not calibration accuracy

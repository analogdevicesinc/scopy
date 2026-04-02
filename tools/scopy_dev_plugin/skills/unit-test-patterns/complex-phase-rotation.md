# Complex: Phase Rotation

## Description

Tests FPGA phase rotation write/readback with tolerance. Includes detection of hardware support — some FPGA configurations don't support phase rotation writes.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains `getPhaseRotation(int channel)` / `setPhaseRotation(int channel, double angle)`.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C3: Phase Rotation Roundtrip
TestFramework.runTest("UNIT.FPGA.PHASE_ROTATION_CH0", function() {
    try {
        var orig = ad9371.getPhaseRotation(0);
        printToConsole("  Original phase ch0: " + orig);
        if (orig === null || orig === "") {
            printToConsole("  SKIP: Phase rotation not available on this hardware");
            return "SKIP";
        }

        // Set to 0 degrees first as baseline
        ad9371.setPhaseRotation(0, 0.0);
        msleep(1000);
        var read0 = parseFloat(ad9371.getPhaseRotation(0));
        printToConsole("  Set 0, read: " + read0);

        // Set to 45 degrees
        ad9371.setPhaseRotation(0, 45.0);
        msleep(1000);
        var read45 = parseFloat(ad9371.getPhaseRotation(0));
        printToConsole("  Set 45, read: " + read45);

        // Check if writes are taking effect
        if (Math.abs(read0) < 2.0 && Math.abs(read45 - 45.0) > 5.0) {
            printToConsole("  SKIP: Phase rotation writes not supported on this FPGA configuration");
            ad9371.setPhaseRotation(0, parseFloat(orig));
            msleep(500);
            return "SKIP";
        }

        if (Math.abs(read45 - 45.0) > 2.0) {
            printToConsole("  FAIL: phase mismatch, expected ~45 got " + read45);
            ad9371.setPhaseRotation(0, parseFloat(orig));
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setPhaseRotation(0, parseFloat(orig));
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Use `return "SKIP"` when feature is not available on hardware
- Wider tolerance for phase angles (2.0 degrees)
- Detect write-not-supported pattern (value doesn't change after write)
- Test per channel — generate one test per channel
- Longer sleep (1000ms) for FPGA operations

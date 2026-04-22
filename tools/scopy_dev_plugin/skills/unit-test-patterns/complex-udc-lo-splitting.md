# Complex: UDC LO Splitting

## Description

Tests Up/Down Converter (UDC) enable/disable with LO frequency interaction. Verifies that UDC can be toggled and LO frequency remains settable.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains `hasUdc()`, `getUdcEnabled()`/`setUdcEnabled()`.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C8: Up/Down Converter LO Splitting
TestFramework.runTest("UNIT.UDC.LO_SPLITTING", function() {
    try {
        if (!ad9371.hasUdc()) {
            printToConsole("  SKIP: UDC hardware not present");
            return "SKIP";
        }

        var origUdc = ad9371.getUdcEnabled();
        var origRxLo = ad9371.getRxLoFrequency();
        printToConsole("  Original UDC=" + origUdc + " RX LO=" + origRxLo);

        // Enable UDC
        ad9371.setUdcEnabled(true);
        msleep(500);
        if (!ad9371.getUdcEnabled()) {
            printToConsole("  FAIL: UDC did not enable");
            return false;
        }

        // Set RX LO
        ad9371.setRxLoFrequency("1000");
        msleep(500);
        var udcLo = ad9371.getRxLoFrequency();
        printToConsole("  UDC enabled, RX LO set to 1000, read: " + udcLo);

        // Disable UDC
        ad9371.setUdcEnabled(false);
        msleep(500);

        // Restore
        ad9371.setRxLoFrequency(origRxLo);
        msleep(500);
        ad9371.setUdcEnabled(origUdc);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Skip when `hasUdc()` returns false
- Save and restore both UDC state and LO frequency
- Test enable, LO set, disable sequence

# Complex: Frequency Tuning

## Description

Tests LO frequency write/readback with unit conversion validation. API typically works in MHz while hardware stores Hz. Uses `testConversion()` helper.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` has getter/setter pairs where the getter returns a scaled value (e.g., MHz) and the widget stores raw values (e.g., Hz).

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
TestFramework.runTest("UNIT.RX.LO_FREQUENCY", function() {
    try {
        return testConversion(
            function() { return ad9371.getRxLoFrequency(); },
            function(v) { ad9371.setRxLoFrequency(v); },
            PHY + "altvoltage0_out/frequency",
            "2500",         // API value: 2500 MHz
            "2500000000",   // Raw value: 2500000000 Hz
            1.0             // tolerance
        );
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Read-Only Frequency Verification

For read-only frequency attributes (RF bandwidth, sampling rate), verify the conversion directly:

```javascript
TestFramework.runTest("UNIT.RX.RF_BANDWIDTH", function() {
    try {
        var raw = ad9371.readWidget(PHY + "voltage0_in/rf_bandwidth");
        if (!raw || raw === "") {
            printToConsole("  SKIP: rf_bandwidth not readable on this hardware");
            return "SKIP";
        }
        var rawHz = parseFloat(raw);
        var apiMHz = parseFloat(ad9371.getRxRfBandwidth());
        var expectedMHz = rawHz / 1e6;
        var diff = Math.abs(apiMHz - expectedMHz);
        if (diff > 0.001) {
            printToConsole("  FAIL: MHz conversion mismatch");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Use `testConversion()` helper for writable frequency attributes
- For read-only frequencies, verify conversion math directly
- Common conversion factor: `1e6` (MHz to Hz)
- Tolerance: 1.0 for frequency values (Hz rounding)

# testConversion — API Conversion Test

## Description

Tests that an API getter/setter pair and the raw widget readback are consistent, validating unit conversion logic (e.g., Hz to MHz, raw to dB). Writes via the API setter, reads back via both the API getter and `readWidget()`, and compares both against expected values.

## When to Use

- API getter returns a scaled/converted value (e.g., MHz) while the widget stores raw values (e.g., Hz)
- API has both getter and setter methods for the same attribute
- Common conversions: Hz/MHz, dB/raw, dBFS/linear, suffix stripping

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `getter` | function | API getter function (e.g., `function() { return api.getRxLoFrequency(); }`) |
| `setter` | function | API setter function (e.g., `function(v) { api.setRxLoFrequency(v); }`) |
| `widgetKey` | string | Full widget key for raw readback |
| `apiVal` | string/number | Value to write via setter (in API units, e.g., MHz) |
| `rawVal` | string/number | Expected raw widget value (in hardware units, e.g., Hz) |
| `tolerance` | number | Acceptable difference for floating-point comparisons (null for exact match) |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testConversion(getter, setter, widgetKey, apiVal, rawVal, tolerance) {
    try {
        var orig = getter();
        printToConsole("  testConversion: orig=" + orig + " apiVal=" + apiVal + " rawVal=" + rawVal);

        // Write via setter
        setter(apiVal);
        msleep(500);

        // Read via getter
        var apiRead = getter();
        printToConsole("  API read=" + apiRead);
        if (tolerance !== undefined && tolerance !== null) {
            var apiDiff = Math.abs(parseFloat(apiRead) - parseFloat(apiVal));
            if (apiDiff > tolerance) {
                printToConsole("  FAIL: API value mismatch, expected=" + apiVal + " got=" + apiRead + " tolerance=" + tolerance);
                setter(orig);
                msleep(500);
                return false;
            }
        } else {
            if (apiRead !== String(apiVal)) {
                printToConsole("  FAIL: API value mismatch, expected=" + apiVal + " got=" + apiRead);
                setter(orig);
                msleep(500);
                return false;
            }
        }

        // Read via readWidget
        var rawRead = ad9371.readWidget(widgetKey);
        printToConsole("  Raw widget read=" + rawRead);
        if (tolerance !== undefined && tolerance !== null) {
            var rawDiff = Math.abs(parseFloat(rawRead) - parseFloat(rawVal));
            if (rawDiff > tolerance) {
                printToConsole("  FAIL: Raw value mismatch, expected=" + rawVal + " got=" + rawRead + " tolerance=" + tolerance);
                setter(orig);
                msleep(500);
                return false;
            }
        } else {
            if (rawRead !== String(rawVal)) {
                printToConsole("  FAIL: Raw value mismatch, expected=" + rawVal + " got=" + rawRead);
                setter(orig);
                msleep(500);
                return false;
            }
        }

        // Restore
        setter(orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testConversion: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
// LO frequency: API in MHz, widget in Hz
TestFramework.runTest("UNIT.RX.LO_FREQUENCY", function() {
    return testConversion(
        function() { return ad9371.getRxLoFrequency(); },
        function(v) { ad9371.setRxLoFrequency(v); },
        PHY + "altvoltage0_out/frequency",
        "2500",         // API value: 2500 MHz
        "2500000000",   // Raw value: 2500000000 Hz
        1.0             // tolerance
    );
});

// TX attenuation: API in positive dB, widget in negative dB
TestFramework.runTest("UNIT.TX.CH0_ATTENUATION", function() {
    return testConversion(
        function() { return ad9371.getTxAttenuation(0); },
        function(v) { ad9371.setTxAttenuation(0, v); },
        PHY + "voltage0_out/hardwaregain",
        "10",    // API value: 10 dB
        "-10",   // Raw value: -10 dB (hardware uses negative)
        0.1      // tolerance
    );
});
```

## Key Requirements

- Both API getter and raw widget are validated in the same test
- `msleep(500)` after every setter call
- Restore original value via the setter (not writeWidget) to maintain conversion consistency
- Use `tolerance` for floating-point values, `null` for exact string match

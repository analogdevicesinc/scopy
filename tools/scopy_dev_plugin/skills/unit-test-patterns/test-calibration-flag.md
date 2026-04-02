# testCalibrationFlag — Calibration Enable Flag Test

## Description

Tests calibration enable/disable flags that use API getter/setter pairs (not IIOWidget read/write). These flags control calibration subsystems (RX QEC, TX QEC, TX LOL, DPD, CLGC, VSWR) and are toggled via dedicated API methods.

## When to Use

- Plugin has `getCalibrateXxxEn()` / `setCalibrateXxxEn()` API methods
- Calibration flags use `MenuOnOffSwitch` (not standard IIOWidget), so `readWidget`/`writeWidget` may not work
- DPD/CLGC/VSWR calibration enable flags (AD9375-specific)

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `uid` | string | Test UID (e.g., `"UNIT.CAL.RX_QEC_EN"`) |
| `getter` | function | API getter (e.g., `function() { return api.getCalibrateRxQecEn(); }`) |
| `setter` | function | API setter (e.g., `function(v) { api.setCalibrateRxQecEn(v); }`) |

## Code Example

From `ad9371_Unit_test.js`:

```javascript
function testCalibrationFlag(uid, getter, setter) {
    TestFramework.runTest(uid, function() {
        try {
            var orig = getter();
            printToConsole("  Original value: " + orig);
            if (orig === null || orig === "") {
                printToConsole("  FAIL: getter returned empty");
                return false;
            }

            setter("1");
            msleep(500);
            var read1 = getter();
            printToConsole("  Set to '1', read back: " + read1);
            if (read1 !== "1") {
                printToConsole("  FAIL: expected '1', got '" + read1 + "'");
                setter(orig);
                msleep(500);
                return false;
            }

            setter("0");
            msleep(500);
            var read0 = getter();
            printToConsole("  Set to '0', read back: " + read0);
            if (read0 !== "0") {
                printToConsole("  FAIL: expected '0', got '" + read0 + "'");
                setter(orig);
                msleep(500);
                return false;
            }

            // Restore
            setter(orig);
            msleep(500);
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });
}
```

## Usage Example

```javascript
// Standard calibration flags
testCalibrationFlag("UNIT.CAL.RX_QEC_EN",
    function() { return ad9371.getCalibrateRxQecEn(); },
    function(v) { ad9371.setCalibrateRxQecEn(v); }
);

testCalibrationFlag("UNIT.CAL.TX_QEC_EN",
    function() { return ad9371.getCalibrateTxQecEn(); },
    function(v) { ad9371.setCalibrateTxQecEn(v); }
);

// AD9375-specific (wrap in isAd9375 guard)
if (isAd9375) {
    testCalibrationFlag("UNIT.CAL.DPD_EN",
        function() { return ad9371.getCalibrateDpdEn(); },
        function(v) { ad9371.setCalibrateDpdEn(v); }
    );
}
```

## Key Requirements

- This helper calls `TestFramework.runTest()` internally (unlike other helpers that return bool)
- Values are strings `"0"` and `"1"`, not booleans
- `msleep(500)` after every setter call
- Restore original value in all exit paths
- Wrap variant-specific flags in skip guards (e.g., `if (isAd9375)`)

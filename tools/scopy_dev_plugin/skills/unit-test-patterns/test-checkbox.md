# testCheckbox — Checkbox/Toggle Widget Test

## Description

Tests a checkbox (boolean toggle) widget by writing "0" and "1" and verifying exact readback. Restores the original value after testing.

## When to Use

- Widget created with `IIOWidgetBuilder::CheckBoxUi`
- Attribute is a binary enable/disable flag

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | string | Full widget key (e.g., `PHY + "adi,rx1-rx2-phase-inversion-en"`) |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testCheckbox(key) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testCheckbox: key=" + key + " orig=" + orig);

        // Write "0"
        ad9371.writeWidget(key, "0");
        msleep(500);
        var read0 = ad9371.readWidget(key);
        printToConsole("  Wrote=0, read=" + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: checkbox mismatch, expected=0 got=" + read0);
            ad9371.writeWidget(key, orig);
            msleep(500);
            return false;
        }

        // Write "1"
        ad9371.writeWidget(key, "1");
        msleep(500);
        var read1 = ad9371.readWidget(key);
        printToConsole("  Wrote=1, read=" + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: checkbox mismatch, expected=1 got=" + read1);
            ad9371.writeWidget(key, orig);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.writeWidget(key, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testCheckbox: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
TestFramework.runTest("UNIT.RX.QEC_TRACKING_EN", function() {
    return testCheckbox(PHY + "voltage0_in/quadrature_tracking_en");
});
```

## Key Requirements

- Values are always strings `"0"` and `"1"`, not booleans
- Exact string comparison with `!==`
- `msleep(500)` after every `writeWidget()` call
- Restore original value on failure and success

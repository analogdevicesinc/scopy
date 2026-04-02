# testRange — Range/Spinbox Widget Test

## Description

Tests a range (spinbox) widget by writing min, max, and mid values and verifying readback with tolerance. Restores the original value after testing.

## When to Use

- Widget created with `IIOWidgetBuilder::EditableUi` or `IIOWidgetBuilder::RangeUi`
- Widget has numeric min/max bounds
- Attribute is writable (not read-only)

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | string | Full widget key (e.g., `PHY + "voltage0_in/hardwaregain"`) |
| `min` | number | Minimum valid value |
| `max` | number | Maximum valid value |
| `mid` | number | Midpoint value for additional verification |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testRange(key, min, max, mid) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testRange: key=" + key + " orig=" + orig);

        var tolerance = 0.01;

        // Write min
        ad9371.writeWidget(key, String(min));
        msleep(500);
        var readMin = ad9371.readWidget(key);
        printToConsole("  Wrote min=" + min + ", read=" + readMin);
        if (Math.abs(parseFloat(readMin) - parseFloat(min)) > tolerance) {
            printToConsole("  FAIL: min mismatch, expected=" + String(min) + " got=" + readMin);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Write max
        ad9371.writeWidget(key, String(max));
        msleep(500);
        var readMax = ad9371.readWidget(key);
        printToConsole("  Wrote max=" + max + ", read=" + readMax);
        if (Math.abs(parseFloat(readMax) - parseFloat(max)) > tolerance) {
            printToConsole("  FAIL: max mismatch, expected=" + String(max) + " got=" + readMax);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Write mid
        ad9371.writeWidget(key, String(mid));
        msleep(500);
        var readMid = ad9371.readWidget(key);
        printToConsole("  Wrote mid=" + mid + ", read=" + readMid);
        if (Math.abs(parseFloat(readMid) - parseFloat(mid)) > tolerance) {
            printToConsole("  FAIL: mid mismatch, expected=" + String(mid) + " got=" + readMid);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Restore
        ad9371.writeWidget(key, String(parseFloat(orig)));
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testRange: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
TestFramework.runTest("UNIT.RX.CH0_HARDWARE_GAIN", function() {
    return testRange(PHY + "voltage0_in/hardwaregain", 0, 30, 15);
});
```

## Key Requirements

- Tolerance is 0.01 by default — adjust if widget has coarser step size
- Always restore original value, even on failure
- `msleep(500)` after every `writeWidget()` call
- Use `String()` conversion for write values, `parseFloat()` for comparisons

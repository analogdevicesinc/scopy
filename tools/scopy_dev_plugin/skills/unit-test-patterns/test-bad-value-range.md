# testBadValueRange — Range Boundary Test

## Description

Tests that a range widget properly clamps out-of-bound values. Writes above the max and below the min, verifying the widget clamps to the boundary value.

## When to Use

- Pair with every `testRange()` call to verify boundary enforcement
- Widget has defined min/max bounds

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | string | Full widget key |
| `min` | number | Minimum valid value |
| `max` | number | Maximum valid value |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testBadValueRange(key, min, max) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testBadValueRange: key=" + key + " orig=" + orig + " validRange=[" + min + "," + max + "]");
        var tolerance = 0.01;

        // Write above max -- expect clamped to max
        var aboveMax = String(parseFloat(max) + 1);
        ad9371.writeWidget(key, aboveMax);
        msleep(500);
        var readAbove = ad9371.readWidget(key);
        printToConsole("  Wrote aboveMax=" + aboveMax + ", read=" + readAbove);
        if (Math.abs(parseFloat(readAbove) - parseFloat(max)) > tolerance) {
            printToConsole("  FAIL: above-max not clamped, expected=" + max + " got=" + readAbove);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Write below min -- expect clamped to min
        var belowMin = String(parseFloat(min) - 1);
        ad9371.writeWidget(key, belowMin);
        msleep(500);
        var readBelow = ad9371.readWidget(key);
        printToConsole("  Wrote belowMin=" + belowMin + ", read=" + readBelow);
        if (Math.abs(parseFloat(readBelow) - parseFloat(min)) > tolerance) {
            printToConsole("  FAIL: below-min not clamped, expected=" + min + " got=" + readBelow);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Restore
        ad9371.writeWidget(key, String(parseFloat(orig)));
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testBadValueRange: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
TestFramework.runTest("UNIT.BAD.RX_HARDWARE_GAIN_RANGE", function() {
    return testBadValueRange(PHY + "voltage0_in/hardwaregain", 0, 30);
});
```

## Key Requirements

- Test value above max uses `max + 1`, below min uses `min - 1`
- Verify the widget clamps (not rejects) — readback should equal the boundary
- `msleep(500)` after every `writeWidget()` call
- Restore original value after test

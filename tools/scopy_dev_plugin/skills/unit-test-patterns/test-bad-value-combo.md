# testBadValueCombo — Invalid Combo Option Test

## Description

Tests that a combo widget rejects an invalid option by verifying the value remains unchanged after writing a bad key.

## When to Use

- Pair with every `testCombo()` call to verify invalid option rejection
- Use a clearly invalid string that cannot match any valid option

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | string | Full widget key |
| `badKey` | string | An invalid option string (e.g., `"INVALID_OPTION_XYZ"`) |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testBadValueCombo(key, badKey) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testBadValueCombo: key=" + key + " orig=" + orig + " badKey=" + badKey);

        // Write invalid key -- expect value unchanged
        ad9371.writeWidget(key, badKey);
        msleep(500);
        var readBack = ad9371.readWidget(key);
        printToConsole("  Wrote badKey=" + badKey + ", read=" + readBack);
        if (readBack !== orig) {
            printToConsole("  FAIL: invalid combo key was accepted, orig=" + orig + " got=" + readBack);
            ad9371.writeWidget(key, orig);
            msleep(500);
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error in testBadValueCombo: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
TestFramework.runTest("UNIT.BAD.GAIN_CONTROL_MODE_COMBO", function() {
    return testBadValueCombo(PHY + "voltage0_in/gain_control_mode", "INVALID_OPTION_XYZ");
});
```

## Key Requirements

- The bad key should be clearly invalid (use `"INVALID_OPTION_XYZ"`)
- Verify value is **unchanged** (equals original), not just "not equal to bad key"
- `msleep(500)` after `writeWidget()` call
- No explicit restore needed (value should be unchanged), but restore if it was accepted

# testReadOnly — Read-Only Widget Test

## Description

Tests a read-only widget by reading its value and verifying it is non-null and non-empty. Does not write any value.

## When to Use

- Widget is display-only (sampling frequency, RSSI, status strings)
- Attribute has no setter or is marked read-only
- Getter-only API methods

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | string | Full widget key (e.g., `PHY + "voltage0_in/sampling_frequency"`) |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testReadOnly(key) {
    try {
        var val = ad9371.readWidget(key);
        printToConsole("  testReadOnly: key=" + key + " val=" + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: readOnly value is empty or null");
            return false;
        }
        printToConsole("  PASS: readOnly value is non-empty: " + val);
        return true;
    } catch (e) {
        printToConsole("  Error in testReadOnly: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
TestFramework.runTest("UNIT.RX.SAMPLING_FREQUENCY", function() {
    return testReadOnly(PHY + "voltage0_in/sampling_frequency");
});
```

## Key Requirements

- No `writeWidget()` call — this is read-only
- Check for both `null` and `""` (empty string)
- No state restoration needed (nothing was modified)

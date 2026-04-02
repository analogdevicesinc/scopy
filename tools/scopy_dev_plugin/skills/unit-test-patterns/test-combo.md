# testCombo — Combo/Dropdown Widget Test

## Description

Tests a combo (dropdown) widget by iterating all valid options, writing each one, and verifying exact string match on readback. Restores the original selection after testing.

## When to Use

- Widget created with `IIOWidgetBuilder::ComboUi`
- Attribute has enumerated options (from `_available` attribute or hardcoded list)

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | string | Full widget key (e.g., `PHY + "ensm_mode"`) |
| `validKeys` | string[] | Array of all valid option strings |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function testCombo(key, validKeys) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testCombo: key=" + key + " orig=" + orig);

        for (var i = 0; i < validKeys.length; i++) {
            ad9371.writeWidget(key, validKeys[i]);
            msleep(500);
            var readBack = ad9371.readWidget(key);
            printToConsole("  Wrote=" + validKeys[i] + ", read=" + readBack);
            if (readBack !== validKeys[i]) {
                printToConsole("  FAIL: combo mismatch, expected=" + validKeys[i] + " got=" + readBack);
                ad9371.writeWidget(key, orig);
                msleep(500);
                return false;
            }
        }

        // Restore
        ad9371.writeWidget(key, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testCombo: " + e);
        return false;
    }
}
```

## Usage Example

```javascript
TestFramework.runTest("UNIT.RX.GAIN_CONTROL_MODE", function() {
    return testCombo(PHY + "voltage0_in/gain_control_mode", ["manual", "automatic", "hybrid"]);
});
```

## Key Requirements

- Use **exact string comparison** (`!==`), not numeric
- Valid options come from EMU XML `_available` attribute or API header
- `msleep(500)` after every `writeWidget()` call
- Restore original value on failure and success

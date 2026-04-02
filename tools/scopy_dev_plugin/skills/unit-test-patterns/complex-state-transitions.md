# Complex: State Transitions

## Description

Tests device state machine transitions (e.g., ENSM mode cycling between radio_off and radio_on) and verifies each state is properly entered.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` contains `getEnsmMode()`/`setEnsmMode()` or similar state machine getter/setter with multiple valid states.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C5: ENSM Mode State Transitions
TestFramework.runTest("UNIT.GLOBAL.ENSM_STATE_TRANSITIONS", function() {
    try {
        var orig = ad9371.getEnsmMode();
        printToConsole("  Original ENSM mode: " + orig);

        ad9371.setEnsmMode("radio_off");
        msleep(500);
        var readOff = ad9371.getEnsmMode();
        printToConsole("  Set radio_off, read: " + readOff);
        if (readOff !== "radio_off") {
            printToConsole("  FAIL: expected 'radio_off', got '" + readOff + "'");
            ad9371.setEnsmMode(orig);
            msleep(500);
            return false;
        }

        ad9371.setEnsmMode("radio_on");
        msleep(500);
        var readOn = ad9371.getEnsmMode();
        printToConsole("  Set radio_on, read: " + readOn);
        if (readOn !== "radio_on") {
            printToConsole("  FAIL: expected 'radio_on', got '" + readOn + "'");
            ad9371.setEnsmMode(orig);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setEnsmMode(orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Test transitions in both directions (off -> on, on -> off)
- Verify each state with exact string comparison
- Restore original state in all exit paths
- Different from `testCombo` — this tests the transition sequence, not just valid values

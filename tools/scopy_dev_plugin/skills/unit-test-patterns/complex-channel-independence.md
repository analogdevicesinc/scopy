# Complex: Channel Independence

## Description

Tests that per-channel attributes are truly independent — changing channel 0 does not affect channel 1 and vice versa.

## API Signature Trigger

Generate this test when the plugin's `*_api.h` has setter methods taking an `int channel` parameter with 2+ channels.

## Code Example

From `ad9371_Unit_test.js`:

```javascript
// C7: Per-Channel Independence
TestFramework.runTest("UNIT.TX.CHANNEL_INDEPENDENCE", function() {
    try {
        var origCh0 = ad9371.getTxAttenuation(0);
        var origCh1 = ad9371.getTxAttenuation(1);
        printToConsole("  Original ch0=" + origCh0 + " ch1=" + origCh1);

        // Set ch0 to 5 dB
        ad9371.setTxAttenuation(0, "5");
        msleep(1000);
        var ch0After = parseFloat(ad9371.getTxAttenuation(0));
        var ch1After = parseFloat(ad9371.getTxAttenuation(1));
        printToConsole("  After setting ch0=5: ch0=" + ch0After + " ch1=" + ch1After);

        if (Math.abs(ch0After - 5.0) > 0.5) {
            printToConsole("  FAIL: ch0 should be ~5");
            ad9371.setTxAttenuation(0, origCh0);
            msleep(500);
            return false;
        }
        if (Math.abs(ch1After - parseFloat(origCh1)) > 0.5) {
            printToConsole("  FAIL: ch1 changed unexpectedly from " + origCh1 + " to " + ch1After);
            ad9371.setTxAttenuation(0, origCh0);
            msleep(500);
            return false;
        }

        // Set ch1 to 10 dB, verify ch0 unchanged
        ad9371.setTxAttenuation(1, "10");
        msleep(1000);
        var ch0Final = parseFloat(ad9371.getTxAttenuation(0));
        var ch1Final = parseFloat(ad9371.getTxAttenuation(1));
        printToConsole("  After setting ch1=10: ch0=" + ch0Final + " ch1=" + ch1Final);

        if (Math.abs(ch0Final - 5.0) > 0.5) {
            printToConsole("  FAIL: ch0 changed when setting ch1");
            ad9371.setTxAttenuation(0, origCh0);
            ad9371.setTxAttenuation(1, origCh1);
            msleep(500);
            return false;
        }
        if (Math.abs(ch1Final - 10.0) > 0.5) {
            printToConsole("  FAIL: ch1 should be ~10");
            ad9371.setTxAttenuation(0, origCh0);
            ad9371.setTxAttenuation(1, origCh1);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setTxAttenuation(0, origCh0);
        ad9371.setTxAttenuation(1, origCh1);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});
```

## Key Requirements

- Save and restore BOTH channels
- Test in both directions: set ch0 verify ch1 unchanged, then set ch1 verify ch0 unchanged
- Use wider tolerance (0.5) for channel independence checks
- Restore both channels in all exit paths

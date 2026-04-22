# runDataDrivenTests — Batch Data-Driven Test Dispatch

## Description

Runs an array of test descriptor objects through the appropriate standard helper function based on the `type` field. This is the preferred way to test sections with 3+ widgets of the same type, reducing boilerplate and ensuring consistency.

## When to Use

- A section has 3 or more widgets that can be tested with standard helpers
- Advanced tool tabs with many similar attributes (e.g., AGC parameters, profile settings)
- Any batch of widgets where individual `TestFramework.runTest()` calls would be repetitive

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `tests` | object[] | Array of test descriptor objects |

### Test Descriptor Object Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `uid` | string | Yes | Test UID (e.g., `"UNIT.CLK.DEVICE_CLOCK_KHZ"`) |
| `attr` | string | Yes | Attribute suffix (appended to PHY prefix) |
| `type` | string | Yes | One of: `"range"`, `"checkbox"`, `"combo"`, `"readonly"` |
| `min` | number | For range | Minimum value |
| `max` | number | For range | Maximum value |
| `mid` | number | For range | Midpoint value |
| `options` | string[] | For combo | Array of valid option strings |

## Code Example

From `ad9371_Basic_Unit_test.js`:

```javascript
function runDataDrivenTests(tests) {
    for (var i = 0; i < tests.length; i++) {
        (function(t) {
            TestFramework.runTest(t.uid, function() {
                if (t.type === "range") return testRange(PHY + t.attr, t.min, t.max, t.mid);
                if (t.type === "checkbox") return testCheckbox(PHY + t.attr);
                if (t.type === "combo") return testCombo(PHY + t.attr, t.options);
                if (t.type === "readonly") return testReadOnly(PHY + t.attr);
                return false;
            });
        })(tests[i]);
    }
}
```

## Usage Example

```javascript
// Advanced tab: CLK Settings
var clkTests = [
    {uid: "UNIT.CLK.DEVICE_CLOCK_KHZ", attr: "adi,clocks-device-clock_khz", type: "range", min: 30720, max: 320000, mid: 122880},
    {uid: "UNIT.CLK.CLKPLL_HS_DIV",    attr: "adi,clocks-clkpll-hs-div",   type: "combo", options: ["4.0", "5.0"]},
    {uid: "UNIT.CLK.CLKPLL_VCO_DIV",   attr: "adi,clocks-clkpll-vco-div",  type: "combo", options: ["1.0", "1.5", "2.0", "3.0"]}
];
runDataDrivenTests(clkTests);

// With conditional skip guard for AD9375
if (isAd9375) {
    var dpdTests = [
        {uid: "UNIT.DPD.DESIRED_GAIN",     attr: "adi,dpd-desired-gain",          type: "range", min: -100, max: 100, mid: 0},
        {uid: "UNIT.DPD.MODEL_VERSION",     attr: "adi,dpd-model-version",         type: "readonly"},
        {uid: "UNIT.DPD.TRACKING_EN",       attr: "adi,dpd-tracking-config-enable", type: "checkbox"}
    ];
    runDataDrivenTests(dpdTests);
} else {
    printToConsole("  SKIP: DPD tests (not AD9375)");
}
```

## Key Requirements

- Use IIFE `(function(t) { ... })(tests[i])` to capture loop variable correctly
- The `PHY` prefix is prepended inside the dispatcher, so `attr` should NOT include it
- Type must exactly match one of: `"range"`, `"checkbox"`, `"combo"`, `"readonly"`
- All standard helper requirements (msleep, state restoration) are enforced by the helpers themselves
- Unrecognized type returns `false` (test failure)

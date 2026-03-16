# RangeUi Widget Patterns

Spinbox/slider widget for numeric attributes with a defined range.

## Pattern 1: optionsAttribute (range from device)

Use when the attribute has a corresponding `*_available` attribute that returns `[min step max]`.

```cpp
// ad936xhelper.cpp:130 — XO Correction with device-reported range
IIOWidget *xoCorrection = IIOWidgetBuilder(globalSettingsWidget)
                .device(dev)
                .attribute("xo_correction")
                .optionsAttribute("xo_correction_available")
                .title("XO Correction")
                .uiStrategy(IIOWidgetBuilder::RangeUi)
                .group(m_group)
                .buildSingle();
layout->addWidget(xoCorrection);
connect(this, &AD936xHelper::readRequested, xoCorrection, &IIOWidget::readAsync);
```

## Pattern 2: optionsValues (hardcoded range)

Use when there is no `*_available` attribute. Provide `"[min step max]"` explicitly.

```cpp
// elnawidget.cpp:57 — eLNA gain with hardcoded range and long info message
IIOWidget *gain = IIOWidgetBuilder(widget)
            .device(m_device)
            .attribute("adi,elna-gain-mdB")
            .uiStrategy(IIOWidgetBuilder::RangeUi)
            .optionsValues("[0 500 31500]")
            .title("LNA Gain (mdB)")
            .infoMessage("These options must have non-zero values only if (1) an external LNA is "
                         "used and (2) the Ext LNA ctrl bits in the Gain Table have been programmed.")
            .group(m_group)
            .buildSingle();
```

## Pattern 3: Negative range (two's complement)

Range values can be negative. Format: `"[min step max]"`.

```cpp
// miscwidget.cpp:58 — DC offset attenuation with negative range
IIOWidget *attenuationHighRange = IIOWidgetBuilder(dcOffsetTracking)
                    .device(m_device)
                    .attribute("adi,dc-offset-attenuation-high-range")
                    .uiStrategy(IIOWidgetBuilder::RangeUi)
                    .optionsValues("[-16 1 15]")
                    .title("")
                    .infoMessage("RX LO > 4 GHz: These bits control the attenuator for "
                                 "the initialization and tracking RF DC offset "
                                 "calibrations. The integrated data shifts by this twos "
                                 "complement value and ranges from -16 to +15.")
                    .group(m_group)
                    .buildSingle();
```

## Pattern 4: MHz ↔ Hz scaling

When the device stores Hz but you want to display MHz, set ALL THREE conversion functions.

```cpp
// ad936x.cpp:206 — RF Bandwidth in MHz (device stores Hz)
IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
                 .channel(voltage0)
                 .attribute("rf_bandwidth")
                 .optionsAttribute("rf_bandwidth_available")
                 .title("RF Bandwidth(MHz)")
                 .uiStrategy(IIOWidgetBuilder::RangeUi)
                 .group(m_group)
                 .buildSingle();

rfBandwidth->setDataToUIConversion([](QString data) {
    return QString::number(data.toDouble() / 1e6, 'f', 6);
});
rfBandwidth->setRangeToUIConversion([](QString data) {
    return QString::number(data.toDouble() / 1e6, 'f', 6);
});
rfBandwidth->setUItoDataConversion([](QString data) {
    return QString::number(data.toDouble() * 1e6, 'f', 0);
});

connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);
```

## Pattern 5: MSPS ↔ SPS scaling

Same pattern as MHz/Hz but for sample rates.

```cpp
// ad936x.cpp:224 — Sampling rate in MSPS (device stores SPS)
IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
                     .channel(voltage0)
                     .attribute("sampling_frequency")
                     .optionsAttribute("sampling_frequency_available")
                     .title("Sampling Rate(MSPS)")
                     .uiStrategy(IIOWidgetBuilder::RangeUi)
                     .group(m_group)
                     .buildSingle();

samplingFrequency->setDataToUIConversion(
    [](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
samplingFrequency->setRangeToUIConversion(
    [](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
samplingFrequency->setUItoDataConversion(
    [](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
```

## Pattern 6: Phase range with std::bind

Use `std::bind` with member functions for conversion when the conversion logic is shared across widgets.

```cpp
// ad9084channel.cpp:124 — NCO Phase with PHASE_RANGE constant
// At top of file: #define PHASE_RANGE "[-180 1 180]"

auto chnNcoPhase = IIOWidgetBuilder(this)
               .channel(m_channel)
               .attribute("channel_nco_phase")
               .uiStrategy(IIOWidgetBuilder::RangeUi)
               .optionsValues(PHASE_RANGE)
               .title("NCO Phase")
               .parent(this)
               .group(m_group)
               .buildSingle();
chnNcoPhase->setUItoDataConversion(
    std::bind(&Ad9084Channel::phaseUItoDS, this, std::placeholders::_1));
chnNcoPhase->setDataToUIConversion(
    std::bind(&Ad9084Channel::phaseDStoUI, this, std::placeholders::_1));
```

## Pattern 7: Factory wrapper method

When creating many range widgets with the same setup, use a helper method.

```cpp
// ad6676.cpp:56 — Factory helper for range widgets
IIOWidget *Ad6676::createRangeWidget(iio_channel *ch, const QString &attr,
                                     const QString &range, const QString &title)
{
    IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
                .device(m_dev)
                .channel(ch)
                .attribute(attr)
                .optionsValues(range)
                .title(title)
                .uiStrategy(IIOWidgetBuilder::RangeUi)
                .group(m_group)
                .buildSingle();

    if(widget) {
        connect(this, &Ad6676::readRequested, widget, &IIOWidget::readAsync);
    }
    return widget;
}
```

## Common Errors

- **Missing `setRangeToUIConversion`**: When using scaling on RangeUi, you MUST set all three: `setDataToUIConversion`, `setRangeToUIConversion`, and `setUItoDataConversion`. Forgetting the range conversion causes the slider range to remain in raw units while the display shows scaled units.
- **Using `optionsAttribute` when no `*_available` exists**: The widget will fail silently. Check if the device actually exposes the `_available` attribute.
- **Using `optionsValues` when `*_available` IS available**: Prefer `optionsAttribute` so the range updates dynamically from the device.
- **Missing `.group(m_group)`**: Widget won't participate in group read/write operations.
- **Missing `readRequested` connection**: Widget will never read its initial value.

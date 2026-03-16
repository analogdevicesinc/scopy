# CheckBoxUi Widget Patterns

Toggle/checkbox widget for boolean or enable/disable attributes.

## Pattern 1: Factory method

When creating many checkboxes with the same setup, use a helper method.

```cpp
// adrv9002.cpp:720 — Factory helper that hides progress bar
IIOWidget *Adrv9002::createCheckboxWidget(iio_channel *ch, const QString &attr, const QString &label)
{
    IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
                .device(m_iio_dev)
                .channel(ch)
                .attribute(attr)
                .title(label)
                .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
                .group(m_group)
                .buildSingle();

    if(widget) {
        connect(this, &Adrv9002::readRequested, widget, &IIOWidget::readAsync);
        widget->showProgressBar(false);
    }
    return widget;
}
```

## Pattern 2: Inline builder

Direct inline creation without a factory method.

```cpp
// ad9084channel.cpp:239 — Test tone enable checkbox
auto chnNcoTestToneEn = IIOWidgetBuilder(this)
                .channel(m_channel)
                .attribute("channel_nco_test_tone_en")
                .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
                .title("Test Tone Enable")
                .parent(this)
                .group(m_group)
                .buildSingle();
chnNcoTestToneEn->showProgressBar(false);
```

## Pattern 3: Multiple checkboxes in a grid

Checkboxes are often placed in grid layouts alongside other widgets.

```cpp
// ad936x.cpp:256 — Tracking enable checkboxes in a grid layout
IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(this)
                    .channel(voltage0)
                    .attribute("quadrature_tracking_en")
                    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
                    .title("Quadrature")
                    .group(m_group)
                    .buildSingle();
layout->addWidget(quadratureTrackingEn, 0, 5);
quadratureTrackingEn->showProgressBar(false);
connect(this, &AD936X::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);
```

## Notes

- **`showProgressBar(false)`**: Almost always called on checkboxes — the progress bar looks awkward on a small toggle widget.
- Checkboxes don't need `optionsAttribute` or `optionsValues` — they always toggle between "0"/"1" (or device-specific on/off values).
- No conversion functions needed for standard boolean attributes.

## Common Errors

- **Forgetting `showProgressBar(false)`**: The progress bar will appear on the checkbox, which looks wrong for a toggle control.
- **Missing `readRequested` connection**: Widget won't read its initial value from the device.
- **Missing `.group(m_group)`**: Widget won't participate in group read/write operations.

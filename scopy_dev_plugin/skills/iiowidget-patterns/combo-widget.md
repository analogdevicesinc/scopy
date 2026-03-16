# ComboUi Widget Patterns

Dropdown/combo box widget for attributes with discrete options.

## Pattern 1: optionsAttribute (options from device)

Use when the attribute has a corresponding `*_available` attribute.

```cpp
// ad6676.cpp:74 — Combo with available attribute via factory helper
IIOWidget *Ad6676::createComboWidget(iio_channel *ch, const QString &attr,
                                     const QString &availableAttr, const QString &title)
{
    IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
                .device(m_dev)
                .channel(ch)
                .attribute(attr)
                .optionsAttribute(availableAttr)
                .title(title)
                .uiStrategy(IIOWidgetBuilder::ComboUi)
                .group(m_group)
                .buildSingle();

    if(widget) {
        connect(this, &Ad6676::readRequested, widget, &IIOWidget::readAsync);
    }
    return widget;
}
```

## Pattern 2: optionsAttribute inline

Simpler inline usage when you don't need a factory method.

```cpp
// ad9084channel.cpp:178 — Test mode with available attribute
auto testMode = IIOWidgetBuilder(this)
            .channel(m_channel)
            .attribute("test_mode")
            .uiStrategy(IIOWidgetBuilder::ComboUi)
            .optionsAttribute("test_mode_available")
            .title("Test Mode")
            .parent(this)
            .group(m_group)
            .buildSingle();
```

## Pattern 3: Custom QMap values (numeric → display mapping)

Use when the device stores numeric values but you want human-readable labels.

```cpp
// rssiwidget.cpp:99 — Numeric keys mapped to descriptive display strings
QMap<QString, QString> *rssiRestartModeOptions = new QMap<QString, QString>();
rssiRestartModeOptions->insert("0", "AGC_in_Fast_Attack_Mode_Locks_the_Gain");
rssiRestartModeOptions->insert("1", "EN_AGC_pin_is_pulled_High");
rssiRestartModeOptions->insert("2", "AD9361_Enters_Rx_Mode");
rssiRestartModeOptions->insert("3", "Gain_Change_Occurs");
rssiRestartModeOptions->insert("4", "SPI_Write_to_Register");
rssiRestartModeOptions->insert("5", "Gain_Change_Occurs_OR_EN_AGC_pin_pulled_High");

// Build space-separated display values string
auto values = rssiRestartModeOptions->values();
QString optionasData = "";
for(int i = 0; i < values.size(); i++) {
    optionasData += " " + values.at(i);
}

IIOWidget *rssiRestartMode = IIOWidgetBuilder(widget)
                 .device(m_device)
                 .attribute("adi,rssi-restart-mode")
                 .uiStrategy(IIOWidgetBuilder::ComboUi)
                 .optionsValues(optionasData)
                 .title("Restart Mode")
                 .group(m_group)
                 .buildSingle();

// Set conversion functions using IIOWidgetUtils helpers
rssiRestartMode->setUItoDataConversion([this, rssiRestartModeOptions](QString data) {
    return IIOWidgetUtils::comboUiToDataConversionFunction(data, rssiRestartModeOptions);
});
rssiRestartMode->setDataToUIConversion([this, rssiRestartModeOptions](QString data) {
    return IIOWidgetUtils::comboDataToUiConversionFunction(data, rssiRestartModeOptions);
});
```

## Pattern 4: BIST tone injection (manual option mapping)

Same custom QMap pattern for binary/enumerated device values.

```cpp
// bistwidget.cpp:65 — BIST PRBS injection point options
QMap<QString, QString> *bistOptions = new QMap<QString, QString>();
bistOptions->insert("0", "Disable");
bistOptions->insert("1", "Injection_Point_TX");
bistOptions->insert("2", "Injection_Point_RX");

auto bistValues = bistOptions->values();
QString bistOptionasData = "";
for(int i = 0; i < bistValues.size(); i++) {
    bistOptionasData += " " + bistValues.at(i);
}

IIOWidget *bistPrbs = IIOWidgetBuilder(widget)
              .device(m_device)
              .attribute("bist_prbs")
              .uiStrategy(IIOWidgetBuilder::ComboUi)
              .optionsValues(bistOptionasData)
              .title("Bist PRBS")
              .group(m_group)
              .buildSingle();

bistPrbs->setUItoDataConversion([this, bistOptions](QString data) {
    return IIOWidgetUtils::comboUiToDataConversionFunction(data, bistOptions);
});
bistPrbs->setDataToUIConversion([this, bistOptions](QString data) {
    return IIOWidgetUtils::comboDataToUiConversionFunction(data, bistOptions);
});
```

## Pattern 5: Complex multi-option mapping (ENSM mode)

When there are many options with complex display names, use the same QMap pattern at larger scale.

```cpp
// ensmmodeclockswidget.cpp:226 — RX port input with 9 options
QMap<QString, QString> *rxPortInputOptions = new QMap<QString, QString>();
rxPortInputOptions->insert("0", "(RX1A_N_&_RX1A_P)_and_(RX2A_N_&_RX2A_P)_enabled");
rxPortInputOptions->insert("1", "(RX1B_N_&_RX1B_P)_and_(RX2B_N_&_RX2B_P)_enabled");
rxPortInputOptions->insert("2", "(RX1C_N_&_RX1C_P)_and_(RX2C_N_&_RX2C_P)_enabled");
// ... up to "8"

auto rxValues = rxPortInputOptions->values();
QString rxOptionasData = "";
for(int i = 0; i < rxValues.size(); i++) {
    rxOptionasData += " " + rxValues.at(i);
}

IIOWidget *rxPortInput = IIOWidgetBuilder(modeWidget)
                 .device(m_device)
                 .attribute("adi,rx-rf-port-input-select")
                 .uiStrategy(IIOWidgetBuilder::ComboUi)
                 .optionsValues(rxOptionasData)
                 .title("RX port input")
                 .group(m_group)
                 .buildSingle();

rxPortInput->setUItoDataConversion([this, rxPortInputOptions](QString data) {
    return IIOWidgetUtils::comboUiToDataConversionFunction(data, rxPortInputOptions);
});
rxPortInput->setDataToUIConversion([this, rxPortInputOptions](QString data) {
    return IIOWidgetUtils::comboDataToUiConversionFunction(data, rxPortInputOptions);
});
```

## Common Errors

- **Forgetting conversion functions with custom values**: When using `.optionsValues()` with a QMap, you MUST set both `setUItoDataConversion` and `setDataToUIConversion`. Without them, the combo shows the display text but writes it literally to the device instead of the mapped value.
- **Using spaces in display values**: MUST use underscores: `"Fast_Attack"` not `"Fast Attack"`. Spaces are used as delimiters in the optionsValues string.
- **Allocating QMap on stack with lambda capture**: The QMap must outlive the widget. Use `new QMap<>()` (heap) or capture by value. In the codebase examples, `new QMap` is used with pointer capture.
- **Missing `.group(m_group)`**: Widget won't participate in group read/write operations.

---
name: iiowidget-patterns
description: How to create IIOWidgets for device attributes using IIOWidgetBuilder. Covers spinbox, combo, checkbox, and read-only patterns. Auto-loads when creating IIOWidgets, mapping attributes, or building plugin UI.
---

# IIOWidget Creation Patterns

Rules for creating `IIOWidget` instances to bind UI controls to IIO device attributes.

## IIOWidgetBuilder Patterns

### Range/Spinbox (with available attribute)

```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
    .device(dev)  // or .channel(ch) if channel-specific
    .attribute("attr")
    .optionsAttribute("attr_available")
    .title("Attribute Display Name")
    .uiStrategy(IIOWidgetBuilder::RangeUi)
    .infoMessage("Hover tooltip text")
    .group(m_group)
    .buildSingle();

connect(this, &MyTool::readRequested, widget, &IIOWidget::readAsync);
```

### Range/Spinbox (with explicit range)

```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
    .device(dev)
    .attribute("attr")
    .optionsValues("MIN MAX STEP")  // e.g. "0 100 1"
    .title("Attribute Display Name")
    .uiStrategy(IIOWidgetBuilder::RangeUi)
    .group(m_group)
    .buildSingle();
```

### Combo Box (with available attribute)

```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
    .device(dev)
    .attribute("attr")
    .optionsAttribute("attr_available")
    .title("Attribute Display Name")
    .uiStrategy(IIOWidgetBuilder::ComboUi)
    .group(m_group)
    .buildSingle();
```

### Combo Box (with custom values)

```cpp
QMap<QString, QString> optionsMap;
optionsMap["value1"] = "Display_Text_1";
optionsMap["value2"] = "Display_Text_2";

QString optionsValues = "Display_Text_1 Display_Text_2";

IIOWidget *widget = IIOWidgetBuilder(parent)
    .device(dev)
    .attribute("attr")
    .optionsValues(optionsValues)
    .title("Attribute Display Name")
    .uiStrategy(IIOWidgetBuilder::ComboUi)
    .group(m_group)
    .buildSingle();

widget->setUItoDataConversion([optionsMap](QString displayValue) {
    return IIOWidgetUtils::comboUiToDataConversionFunction(displayValue, &optionsMap);
});
widget->setDataToUIConversion([optionsMap](QString attrValue) {
    return IIOWidgetUtils::comboDataToUiConversionFunction(attrValue, &optionsMap);
});
```

### Checkbox

```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
    .device(dev)
    .attribute("attr")
    .title("Attribute Display Name")
    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
    .group(m_group)
    .buildSingle();
```

### Read-Only Status (with 2-failure stop timer)

```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
    .channel(ch)
    .attribute("status_attr")
    .title("Status Display Name")
    .group(m_group)
    .buildSingle();
widget->setEnabled(false);

QTimer *timer = new QTimer(parent);
int *failCount = new int(0);
connect(timer, &QTimer::timeout, widget, &IIOWidget::readAsync);
connect(widget, &IIOWidget::currentStateChanged, timer,
    [timer, failCount](IIOWidget::State state, QString) {
        if(state == IIOWidget::Error) {
            (*failCount)++;
            if(*failCount >= 2) timer->stop();
        } else if(state == IIOWidget::Correct) {
            *failCount = 0;
        }
    });
timer->start(1000);
```

## Scaling Conversions

When an attribute needs unit conversion (e.g., Hz to MHz), set ALL THREE functions:

```cpp
widget->setDataToUIConversion([](QString data) {
    return QString::number(data.toDouble() / 1e6, 'f', 6);
});
widget->setRangeToUIConversion([](QString data) {
    return QString::number(data.toDouble() / 1e6, 'f', 6);
});
widget->setUItoDataConversion([](QString data) {
    return QString::number(data.toDouble() * 1e6, 'f', 0);
});
```

## Critical Requirements

1. **Every widget MUST connect** `readRequested` -> `readAsync` (caller's responsibility)
2. **RangeUi with scaling needs ALL THREE** conversion functions
3. **Read-only timer widgets** use `currentStateChanged` signal (NOT `readFailed`/`readSuccess` — those don't exist)
4. **Always include `.group(m_group)`** in the builder chain
5. **Use underscores** in display values for custom combos: `"Fast_Attack"` not `"Fast Attack"`

## Decision Tree

**Range/Spinbox**: Has `*_available` attr? YES -> use `.optionsAttribute()`. NO -> use `.optionsValues("MIN MAX STEP")`

**Combo**: Has `*_available` attr? YES -> `.optionsAttribute()`. NO -> custom values with `.optionsValues()` + conversion functions

## Available UI Strategies

Each strategy has a dedicated file with real codebase examples:

| Strategy | File | Use Case |
|----------|------|----------|
| `RangeUi` | `range-widget.md` | Numeric attributes with min/max/step (spinbox/slider) |
| `ComboUi` | `combo-widget.md` | Discrete options (dropdown) |
| `CheckBoxUi` | `checkbox-widget.md` | Boolean/enable toggle |
| `EditableUi` | `editable-widget.md` | Free-form text input (no range) |
| Read-only | `readonly-widget.md` | Disabled display with optional timer polling |
| `TemperatureUi` | `temperature-widget.md` | Temperature with critical threshold warnings |

# EditableUi Widget Patterns

Free-form text input widget for attributes without a constrained range.

## When to Use EditableUi vs RangeUi

- **EditableUi**: Use when the attribute has no `*_available` range and no meaningful min/max/step. The user types a value directly.
- **RangeUi**: Use when you know the valid range (either from `*_available` or hardcoded). Provides a slider/spinbox with constraints.

## Pattern 1: std::bind conversion (no group registration)

Use `std::bind` to bind member functions as conversion callbacks.

```cpp
// txtone.cpp:57 — Frequency with MHz↔Hz conversion, no group
m_frequency = IIOWidgetBuilder(this)
          .channel(m_node->getChannel())
          .attribute("frequency")
          .uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
          .parent(this)
          .buildSingle();
m_frequency->setUItoDataConversion(
    std::bind(&TxTone::frequencyUItoDS, this, std::placeholders::_1));
m_frequency->setDataToUIConversion(
    std::bind(&TxTone::frequencyDStoUI, this, std::placeholders::_1));
```

Note: `IIOWidgetBuilder::UIS::EditableUi` is equivalent to `IIOWidgetBuilder::EditableUi`.

## Pattern 2: Member function binding (with group)

Same pattern but with group registration for coordinated read/write.

```cpp
// ad9084channel.cpp:82 — ADC frequency with group registration
m_frequencyWidget = IIOWidgetBuilder(this)
                .channel(m_channel)
                .attribute("adc_frequency")
                .uiStrategy(IIOWidgetBuilder::EditableUi)
                .title("ADC Frequency (MHz)")
                .parent(this)
                .group(m_group)
                .buildSingle();
m_iioWidgetGroupList.value(ADC_FREQUENCY)->add(m_frequencyWidget);

m_frequencyWidget->setUItoDataConversion(
    std::bind(&Ad9084Channel::frequencyUItoDS, this, std::placeholders::_1));
m_frequencyWidget->setDataToUIConversion(
    std::bind(&Ad9084Channel::frequencyDStoUI, this, std::placeholders::_1));
```

## Notes

- EditableUi does NOT need `setRangeToUIConversion` — there is no range/slider to convert. Only `setDataToUIConversion` and `setUItoDataConversion` are needed.
- When using `std::bind`, don't forget `std::placeholders::_1` for the QString parameter.
- The `parent(this)` call is optional but recommended for proper Qt object ownership.

## Common Errors

- **Using EditableUi when RangeUi is appropriate**: If you know the range, use RangeUi for better UX (constraints, slider).
- **Missing conversion functions**: If the attribute stores raw values (Hz) but the title says "MHz", you must set conversion functions.
- **Forgetting `std::placeholders::_1`**: The conversion function receives a QString parameter — the placeholder must be included.

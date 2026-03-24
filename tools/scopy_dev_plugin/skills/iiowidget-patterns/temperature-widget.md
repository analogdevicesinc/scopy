# TemperatureUi Widget Patterns

Specialized widget for displaying temperature with critical threshold warnings.

## When to Use TemperatureUi vs Read-Only

- **TemperatureUi**: Use for temperature attributes where you want visual warnings at critical thresholds (color changes, warning messages).
- **Read-only**: Use for temperature values where threshold warnings are not needed.

## Pattern 1: Critical temperature with warning offset

The full pattern with `TemperatureGuiStrategy` configuration for critical temperature and warning offset.

```cpp
// adrv9002.cpp:295 — Temperature widget with 80°C critical, 75°C warning
iio_channel *tempCh = iio_device_find_channel(m_iio_dev, "temp0", false);
if(tempCh) {
    IIOWidget *tempWidget = IIOWidgetBuilder(this)
                    .device(m_iio_dev)
                    .channel(tempCh)
                    .attribute("input")
                    .uiStrategy(IIOWidgetBuilder::TemperatureUi)
                    .title("Temperature")
                    .group(m_group)
                    .buildSingle();

    if(tempWidget) {
        tempWidget->showProgressBar(false);

        // Access the TemperatureGuiStrategy to set critical temperature
        auto *tempStrategy = dynamic_cast<TemperatureGuiStrategy *>(
            tempWidget->getUiStrategy());
        if(tempStrategy) {
            tempStrategy->setCriticalTemperature(
                80.0, "ADRV9002 temperature critical! Risk of thermal shutdown.");
            tempStrategy->setWarningOffset(5.0);  // Warn at 75°C (80°C - 5°C)
        }

        connect(this, &Adrv9002::readRequested, tempWidget, &IIOWidget::readAsync);
        layout->addWidget(tempWidget);
    }
}
```

## Key Details

- **`getUiStrategy()`**: Returns the base `GuiStrategy*`. Must be `dynamic_cast` to `TemperatureGuiStrategy*` to access temperature-specific methods.
- **`setCriticalTemperature(double temp, QString message)`**: Sets the temperature at which the widget shows a critical warning. The message appears as a tooltip or alert.
- **`setWarningOffset(double offset)`**: Sets how many degrees before critical to start showing a warning state. E.g., with critical=80 and offset=5, warning starts at 75°C.
- **`showProgressBar(false)`**: Temperature widgets typically hide the progress bar.

## Common Errors

- **Forgetting `dynamic_cast`**: `getUiStrategy()` returns `GuiStrategy*`, not `TemperatureGuiStrategy*`. You must cast to access `setCriticalTemperature` and `setWarningOffset`.
- **Not checking the cast result**: Always check `if(tempStrategy)` after the `dynamic_cast` — it returns `nullptr` if the cast fails.
- **Missing temperature channel check**: Always verify the `temp0` channel exists with `if(tempCh)` before building the widget.

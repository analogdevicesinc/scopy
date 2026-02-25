# Prompt: Integrate IIOWidgetGroup into a Scopy Plugin

## Goal

Modify an existing Scopy plugin so that all its `IIOWidget` instances are registered in a centralized `IIOWidgetGroup`. This enables key-based lookup of any widget at runtime, which is a prerequisite for later generating a JS API class.


## Prerequisites

- The plugin already creates `IIOWidget` instances via `IIOWidgetBuilder(...).buildSingle()`.
- The `iio-widgets` library provides `IIOWidgetGroup` (header: `<iio-widgets/iiowidgetgroup.h>`).

## Reference Implementation

The AD936x plugin serves as the canonical example:
- `packages/ad936x/plugins/ad936x/include/ad936x/ad936xplugin.h`
- `packages/ad936x/plugins/ad936x/src/ad936xplugin.cpp`
- `packages/ad936x/plugins/ad936x/src/ad936x/ad936x.cpp`
- `packages/ad936x/plugins/ad936x/src/gainwidget.cpp`
- `iio-widgets/include/iio-widgets/iiowidgetgroup.h`

---

## Step-by-Step Instructions

### 1. Forward-declare IIOWidgetGroup in the plugin header

Add a forward declaration **outside** the plugin's own namespace, in the `scopy` namespace:

```cpp
namespace scopy {
class IIOWidgetGroup;
}
```

Then add a member variable inside the plugin class (private section):

```cpp
IIOWidgetGroup *m_widgetGroup = nullptr;
```

**Example (ad936xplugin.h):**
```cpp
namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad936x {
class SCOPY_AD936X_EXPORT Ad936xPlugin : public QObject, public PluginBase
{
    Q_OBJECT
    SCOPY_PLUGIN;
    // ...
private:
    IIOWidgetGroup *m_widgetGroup = nullptr;
};
}
```

### 2. Create the group in `onConnect()`

In the plugin's `onConnect()` method, create the group **before** constructing any sub-components that build widgets:

```cpp
#include <iio-widgets/iiowidgetgroup.h>

bool YourPlugin::onConnect()
{
    Connection *conn = ConnectionProvider::open(m_param);
    if(!conn) {
        return false;
    }

    m_widgetGroup = new IIOWidgetGroup(this);

    // Now create sub-components, passing m_widgetGroup to each
    YourTool *tool = new YourTool(conn->context(), m_widgetGroup);
    m_toolList[0]->setTool(tool);
    m_toolList[0]->setEnabled(true);
    // ...

    return true;
}
```

### 3. Delete the group in `onDisconnect()`

In `onDisconnect()`, delete the group **after** deleting the tool widgets (since widgets may reference the group):

```cpp
bool YourPlugin::onDisconnect()
{
    // Delete tools first
    for(auto &tool : m_toolList) {
        tool->setEnabled(false);
        tool->setRunning(false);
        tool->setRunBtnVisible(false);
        QWidget *w = tool->tool();
        if(w) {
            tool->setTool(nullptr);
            delete(w);
        }
    }

    // Then delete the group
    if(m_widgetGroup) {
        delete m_widgetGroup;
        m_widgetGroup = nullptr;
    }

    ConnectionProvider::close(m_param);
    return true;
}
```

### 4. Pass the group pointer to all sub-components

Every class that builds `IIOWidget` instances must receive the `IIOWidgetGroup *` via its constructor and store it as a member.

**Constructor signature pattern:**
```cpp
// Header
class YourTool : public QWidget
{
public:
    explicit YourTool(iio_context *ctx, IIOWidgetGroup *mgr, QWidget *parent = nullptr);
private:
    IIOWidgetGroup *m_mgr = nullptr;
};

// Source
YourTool::YourTool(iio_context *ctx, IIOWidgetGroup *mgr, QWidget *parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    // ...
}
```

If a sub-component creates further nested widgets (e.g., `GainWidget` inside `AD936X`), pass the group pointer down through each level.

### 5. Add `.group(m_mgr)` to every IIOWidgetBuilder chain

This is the core change. For **every** `IIOWidgetBuilder` call, insert `.group(m_mgr)` into the builder chain **before** `.buildSingle()`.

**Before:**
```cpp
IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
    .channel(voltage0)
    .attribute("rf_bandwidth")
    .optionsAttribute("rf_bandwidth_available")
    .title("RF Bandwidth(MHz)")
    .uiStrategy(IIOWidgetBuilder::RangeUi)
    .buildSingle();
```

**After:**
```cpp
IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
    .channel(voltage0)
    .attribute("rf_bandwidth")
    .optionsAttribute("rf_bandwidth_available")
    .title("RF Bandwidth(MHz)")
    .uiStrategy(IIOWidgetBuilder::RangeUi)
    .group(m_mgr)
    .buildSingle();
```

The `.group()` call registers the widget in the group using an auto-generated key derived from the device/channel/attribute path (e.g., `"ad9361-phy/voltage0_in/rf_bandwidth"`).

### 6. Include the group header in source files

Any `.cpp` file that uses the group must include:
```cpp
#include <iio-widgets/iiowidgetgroup.h>
```

---

## Widget Key Format

When `.group(m_mgr)` is called, the key is automatically generated from the `IIOWidgetFactoryRecipe`. The format is:

- **Device-level attribute:** `"device-name//attribute-name"` (double slash, no channel)
- **Channel-level attribute:** `"device-name/channelN_dir/attribute-name"`

Examples:
- `"ad9361-phy//ensm_mode"` - device-level attribute
- `"ad9361-phy/voltage0_in/rf_bandwidth"` - input channel attribute
- `"ad9361-phy/voltage0_out/rf_bandwidth"` - output channel attribute
- `"ad9361-phy/altvoltage0_out/frequency"` - LO frequency

---

## Checklist

- [ ] Forward-declared `IIOWidgetGroup` in the `scopy` namespace in plugin header
- [ ] Added `IIOWidgetGroup *m_widgetGroup = nullptr;` as private member
- [ ] Created group in `onConnect()` before sub-component construction
- [ ] Deleted group in `onDisconnect()` after tool cleanup
- [ ] Updated all sub-component constructors to accept `IIOWidgetGroup*`
- [ ] Stored group as member `m_mgr` in each sub-component
- [ ] Added `.group(m_mgr)` to **every** `IIOWidgetBuilder` chain before `.buildSingle()`
- [ ] Added `#include <iio-widgets/iiowidgetgroup.h>` where needed
- [ ] Verified that deeply nested widget-building classes also receive the group

## Verification

After applying the changes:
1. Build the plugin and verify no compilation errors.
2. Connect to a device and check logs for any widget registration warnings.
3. Optionally, call `m_widgetGroup->keys()` from a debug breakpoint or log statement to confirm all widgets are registered with the expected keys.

---
name: scopy-plugin-patterns
description: Standard scopy plugin patterns including ToolTemplate, refresh buttons, sections, plugin lifecycle, and architecture. Auto-loads when creating plugin scaffolds, tool classes, or section layouts.
---

# Scopy Plugin Patterns

## Plugin Class Structure

```cpp
class MyPlugin : public QObject, PluginBase {
    SCOPY_PLUGIN
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.adi.Scopy.PluginBase")
    Q_INTERFACES(scopy::PluginBase)

public:
    bool compatible(QString m_param, QString category) override;
    void loadToolList() override;
    bool loadIcon() override;
    void initMetadata() override;
    bool onConnect() override;
    bool onDisconnect() override;

private:
    IIOWidgetGroup *m_widgetGroup = nullptr;
};
```

## Plugin Lifecycle

- **`compatible()`**: Device detection using `iio_context_find_device(conn->context(), "name")`
- **`onConnect()`**: Create `IIOWidgetGroup`, create tools, set tool on `m_toolList`
- **`onDisconnect()`**: Delete tools, delete widget group, `ConnectionProvider::close(m_param)`
- **`initMetadata()`**: JSON with priority, category, exclude

## Tool Constructor Signature

```cpp
explicit MyTool(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
```

## ToolTemplate + Refresh Button

```cpp
m_tool = new ToolTemplate(this);
m_refreshButton = new AnimatedRefreshBtn(false, this);
m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);
```

Refresh button triggers `readRequested()` via `QtConcurrent::run`.

## Scopy Package Structure

```
scopy/packages/<name>/
├── manifest.json.cmakein
├── CMakeLists.txt
├── emu-xml/
└── plugins/<name>/
    ├── src/                  # Implementation files
    ├── include/<name>/       # Header files
    ├── test/                 # Unit tests
    └── CMakeLists.txt
```

## Function Mapping (from legacy iio-oscilloscope)

| iio-oscilloscope | Scopy | Purpose |
|---|---|---|
| `identify()` | `compatible()` | Device detection |
| `init()` | `onConnect()` | Create tools on connection |
| `destroy()` | `onDisconnect()` | Cleanup on disconnect |
| `iio_widget` helpers | `IIOWidget` / `IIOWidgetBuilder` | Attribute binding |
| Glade XML | Programmatic Qt code | UI definition |

## Advanced Tool Pattern (for plugins with `adi,*` debug attributes)

- QStackedWidget + sidebar navigation buttons
- Each sub-tab is its own widget class
- Sub-tabs use IIOWidgetBuilder for widget creation
- Use `MenuSectionCollapseWidget` for collapsible sections in sub-tabs

## Architecture Decision Tree

```
Has debug/advanced attributes (adi,*)?
├── YES → Basic tool + Advanced tool (QStackedWidget + sub-tabs)
└── NO → Single tool only

Shares devices with another plugin?
├── YES → Multiple tool variants (detect in onConnect)
└── NO → Single tool (most common)
```

## Section Widget Creation

```cpp
QWidget *section = new QWidget(parent);
Style::setBackgroundColor(section, json::theme::background_primary);
Style::setStyle(section, style::properties::widget::border_interactive);
QLabel *title = new QLabel("Section Title", section);
Style::setStyle(title, style::properties::label::menuBig);
```

## Logging Categories

```cpp
// Header
Q_DECLARE_LOGGING_CATEGORY(CAT_MYPLUGIN)

// Source
Q_LOGGING_CATEGORY(CAT_MYPLUGIN, "MyPlugin")

// Usage
qDebug(CAT_MYPLUGIN) << "Message";
```

## Full Reference

See `reference.md` in this directory for complete lifecycle code templates.

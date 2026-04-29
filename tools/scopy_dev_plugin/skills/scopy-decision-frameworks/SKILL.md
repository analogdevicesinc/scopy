---
name: scopy-decision-frameworks
description: Decision trees for common Scopy architectural choices - plugin vs core, tool vs section, widget type selection, sync vs async, GNU Radio vs direct, package organization, and API exposure. Loaded when making design decisions.
---

# Scopy Decision Frameworks

## Plugin vs Core Library

```
Is this feature device-specific?
├── YES → Plugin (in the device's package)
└── NO
    ├── Is it used by 3+ plugins?
    │   ├── YES → Core library (common, gui, iioutil, or pluginbase)
    │   └── NO
    │       ├── Is it optional/removable?
    │       │   ├── YES → Plugin (in generic-plugins package)
    │       │   └── NO → Core library
    │       └── Is it UI-only?
    │           ├── YES → gui library
    │           └── NO → common or iioutil
```

**Guidelines:**
- If it touches IIO directly → `iioutil` or `iio-widgets`
- If it's a reusable Qt widget → `gui`
- If it's plugin infrastructure → `pluginbase`
- If it manages packages → `pkg-manager`
- If none of the above → `common`

## New Package vs Extend Existing

```
Does this support a new device family?
├── YES → New package under packages/
└── NO
    ├── Does the target device already have a package?
    │   ├── YES → Add plugin to existing package
    │   └── NO
    │       ├── Is it a reference design using existing devices?
    │       │   ├── YES → New package (like cn0511, daq2, fmcomms11)
    │       │   └── NO → generic-plugins package
    │       └── Is it cross-device utility?
    │           └── YES → generic-plugins package
```

## Tool vs Section

```
Does this feature have its own independent workflow?
├── YES → New tool (new ToolMenuEntry in loadToolList())
│         User sees it as a separate menu item
└── NO
    ├── Is it part of an existing tool's workflow?
    │   ├── YES → New section in existing tool
    │   │         (MenuSectionCollapseWidget or tab)
    │   └── NO
    │       ├── Is it configuration/settings?
    │       │   ├── YES → Preferences page (loadPreferencesPage())
    │       │   └── NO → New tool
    │       └── Is it a one-time action?
    │           ├── YES → Button or menu action in existing tool
    │           └── NO → New tool
```

## IIOWidget vs Custom Widget

```
Does it map to a single IIO attribute?
├── YES
│   ├── Enumerated values? → IIOWidgetBuilder::ComboUi
│   ├── Numeric range? → IIOWidgetBuilder::RangeUi
│   ├── Boolean on/off? → IIOWidgetBuilder::CheckBoxUi or SwitchUi
│   ├── Free-form text? → IIOWidgetBuilder::EditableUi
│   ├── Temperature with thresholds? → IIOWidgetBuilder::TemperatureUi
│   └── Read-only status? → IIOWidgetBuilder with read-only pattern + timer polling
└── NO
    ├── Multi-attribute interaction? → Custom QWidget
    │   (e.g., gain + frequency coupled control)
    ├── Complex visualization? → Custom QWidget with QWT plots
    ├── File selection/dialog? → Custom QWidget
    └── No IIO attribute at all? → Standard Qt widget
```

**IIOWidgetBuilder decision sub-tree:**
```
Has available-attribute (enum options)?
├── YES → Use .optionsAttribute("attr_available") with ComboUi
└── NO
    ├── Has numeric range?
    │   ├── Range from optionsAttribute → .optionsAttribute() with RangeUi
    │   └── Hardcoded range → .optionsValues("min max step") with RangeUi
    └── No constraints → EditableUi (accepts any string)
```

## Sync vs Async IIO Access

```
What's the access pattern?
├── One-time read at startup → Direct iio_*_attr_read() in onConnect()
├── Periodic polling (device status) → CyclicalTask
│   └── Set period based on data rate needs (default 5000ms)
├── Connection health monitoring → PingTask subclass
│   └── Implement ping() to check device alive
├── User-triggered read/write → IIOWidget with read()/writeAsync()
│   └── Group with IIOWidgetGroup for batch operations
├── Continuous data streaming → GNU Radio pipeline (gr-util)
└── Event-driven updates → MessageBroker subscription
```

## GNU Radio vs Direct Data Processing

```
Is this continuous data streaming?
├── YES
│   ├── Needs signal processing (FFT, filtering, decimation)?
│   │   ├── YES → GNU Radio pipeline with gr-util
│   │   └── NO → Direct IIO buffer reads
│   ├── Needs real-time plotting?
│   │   ├── YES → GNU Radio → QWT plot sink
│   │   └── NO → Direct IIO → custom processing
│   └── Multiple channels with different processing?
│       └── YES → GNU Radio flowgraph (natural multi-path support)
└── NO
    ├── Single attribute read/write → IIOWidget or direct IIO call
    ├── Batch attribute operations → IIOWidgetGroup
    └── Register-level access → Direct IIO device attribute read/write
```

## API Exposure Decision

```
Does this feature need automated testing?
├── YES → Create *_api.h / *_api.cpp
│   ├── Inherit from ApiObject
│   ├── Q_INVOKABLE on all public methods
│   ├── friend class in plugin header
│   ├── Register with ScopyJS in onConnect()
│   └── Delete at start of onDisconnect()
└── NO
    ├── Is it UI-only with no testable state?
    │   └── YES → No API needed
    ├── Might it need scripting access later?
    │   └── YES → Create API now (cheaper than retrofitting)
    └── Is it infrastructure/internal?
        └── YES → No API needed
```

## Testing Strategy Decision

```
What type of feature is this?
├── Core library change → Qt unit tests in library's test/ dir
├── Plugin with IIO attributes
│   ├── Unit tests → Qt Test (mock IIO context if possible)
│   ├── Integration tests → JS HITL tests (claude_scopy_dev_plugin branch)
│   ├── Manual tests → RST test docs with UIDs and RBP levels
│   └── API class → Required for JS automation
├── UI-only change
│   ├── Visual tests → JS VisualTests.js with supervisedCheck()
│   └── Manual tests → RST test docs (Category C)
└── Build/CI change → CI pipeline validation
```

## Communication Pattern Decision

```
Who needs to know about this event?
├── Same plugin only → Qt signals/slots (direct connection)
├── Other plugins need to react
│   ├── Known recipients → MessageBroker with specific topic
│   └── Broadcast to all → MessageBroker with "broadcast" topic
├── Core needs to react → Plugin signals (connectDevice, disconnectDevice, etc.)
├── UI needs to update → Qt signals/slots + property binding
└── External tools need access → ScopyJS API registration
```

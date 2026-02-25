# Scopy Plugin Generator Agent

You are an expert Scopy plugin generator. Given an `iio_info` text file, you produce a complete, buildable Scopy plugin package. You follow the ad936x plugin as the canonical reference for all patterns.

---

## Reference Files

Read these files before generating any code:

- @packages/ad936x/plugins/ad936x/src/ad936xplugin.cpp
- @packages/ad936x/plugins/ad936x/include/ad936x/ad936xplugin.h
- @packages/ad936x/plugins/ad936x/src/ad936x/ad936x.cpp
- @packages/ad936x/plugins/ad936x/include/ad936x/ad936x/ad936x.h
- @packages/ad936x/plugins/ad936x/src/ad936xhelper.cpp
- @packages/ad936x/plugins/ad936x/include/ad936x/ad936xhelper.h
- @packages/ad936x/plugins/ad936x/include/ad936x/ad936x_api.h
- @packages/ad936x/plugins/ad936x/src/ad936x_api.cpp
- @packages/ad936x/plugins/ad936x/CMakeLists.txt
- @packages/ad936x/CMakeLists.txt
- @iio-widgets/src/iiowidgetgroup.cpp
- @tools/prompts/iiowidgetgroup_integration.md

---

## A. iio_info Parsing Rules

Parse the iio_info file using these exact patterns.

### Context Attributes
```
IIO context has N attributes:
	key: value
```
Each line indented with one tab is a context attribute.

### Devices
```
	iio:deviceN: device-name (buffer capable)
```
- Device ID: `iio:deviceN`
- Device name: text after `: ` and before ` (` or end of line
- Buffer capable: present if `(buffer capable)` suffix exists
- A device may also have a label: `iio:deviceN: device-name (label: some-label)`

### Channels
```
		channelId: label (input|output)
		channelId: label (input|output, index: N, format: XXX)
```
- Channel ID: text before first `:`
- Label: text between `: ` and ` (`
- Direction: `input` or `output`
- Optional: `index` and `format` for buffer-capable channels
- Channels with empty labels show as `channelId:  (direction)` (note double space)

### Channel Attributes
```
			N channel-specific attributes found:
				attr  N: name value: XXX
				attr  N: name ERROR: message (code)
```
- Attribute name: text between `attr  N: ` and ` value:` (or ` ERROR:`)
- Value: everything after `value: ` on that line (may be multi-line for some attributes)
- ERROR attributes: the attribute exists but reading it returned an error

### Device Attributes
```
		N device-specific attributes found:
				attr  N: name value: XXX
```
Same format as channel attributes but under the device-specific section.

### Debug Attributes
```
		N debug attributes found:
				debug attr  N: name value: XXX
```
Same format but prefixed with `debug attr`.

### Multi-line Values
Some attributes (like `gain_table_config`, `rssi_gain_step_error`, `pseudorandom_err_check`) have values that span multiple lines. The value continues until the next `attr` or `debug attr` line.

### `_available` Suffix Pairing
When an attribute named `foo_available` exists alongside `foo`, the `_available` attribute provides the options for `foo`:
- `[min step max]` format: use `RangeUi`
- `val1 val2 val3 ...` format: use `ComboUi`

---

## B. Attribute-to-Widget Strategy Mapping

Use this decision table to choose the `IIOWidgetBuilder` UI strategy for each attribute.

### Decision Table

| Priority | Condition | Has `_available`? | `_available` Format | UI Strategy | Notes |
|----------|-----------|-------------------|---------------------|-------------|-------|
| 0 | Attribute value is ERROR | - | - | **SKIP** | Do not create a widget |
| 1 | Attribute is `_available`-only (no base attr) | - | - | **SKIP** | Only used as options for another attr |
| 2 | Name is `filter_fir_config` | - | - | **SKIP** | Complex multi-line config |
| 3 | Name is `gain_table_config` | - | - | **SKIP** | Complex multi-line config |
| 4 | Name is `direct_reg_access` | - | - | **SKIP** | Debug-only register access |
| 5 | Name is `waiting_for_supplier` | - | - | **SKIP** | Internal kernel attr |
| 6 | Name is `label` | - | - | **SKIP** | Informational only |
| 7 | Any attr | Yes | `[min step max]` | `RangeUi` | Range slider with min/step/max |
| 8 | Any attr | Yes | `val1 val2 ...` | `ComboUi` | Dropdown with discrete options |
| 9 | Name ends with `_en` | No | - | `CheckBoxUi` | Boolean toggle |
| 10 | Channel type is `temp*` input | No | - | `EditableUi` | Temperature reading |
| 11 | Name is `rssi` | No | - | `EditableUi` + `setEnabled(false)` | Read-only display |
| 12 | Name is `*_path_rates` | No | - | `EditableUi` + `setEnabled(false)` | Read-only display |
| 13 | Name is `rssi_gain_step_error` | No | - | `EditableUi` + `setEnabled(false)` | Read-only display |
| 14 | Default | No | - | `EditableUi` | Editable text field |

### Unit Conversions

Apply these conversions when the attribute name matches:

| Attribute Name | Raw Unit | Display Unit | Display Title Suffix | Conversion Factor |
|----------------|----------|--------------|----------------------|-------------------|
| `frequency` | Hz | MHz | (MHz) | 1e6 |
| `rf_bandwidth` | Hz | MHz | (MHz) | 1e6 |
| `sampling_frequency` | SPS | MSPS | (MSPS) | 1e6 |
| `hardwaregain` | dB (with " dB" suffix) | dB | (dB) | Parse: split on space, take first |

When a conversion is needed, set these on the IIOWidget:
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

For `hardwaregain` (which has " dB" suffix in the value):
```cpp
widget->setDataToUIConversion([](QString data) {
	auto result = data.split(" ");
	return result.first();
});
```

---

## C. Device Role Classification

When the iio_info contains multiple devices, classify each device's role:

| Device Name Pattern | Role | Tool Organization |
|---------------------|------|-------------------|
| `*-phy` (e.g., `ad9361-phy`) | Primary PHY | Main controls tool — device-level attrs, RX channels, TX channels |
| `cf-*-dds-core-*` (e.g., `cf-ad9361-dds-core-lpc`) | DDS | Optional DDS tool (often handled by separate DDS plugin) |
| `cf-*-lpc` or `cf-*-hpc` (e.g., `cf-ad9361-lpc`) | ADC Frontend | Optional ADC tool (often handled by separate plugin) |
| `xadc` | System Monitor | Optional section showing voltage rails and temperature |
| `one-bit-adc-dac` | GPIO | Usually skip unless user requests |
| `adi-iio-fakedev` | TDD | Usually skip unless user requests |
| Any device with debug attrs | Advanced | Separate "Advanced" tool with debug attributes |

The primary PHY device is the main target for the plugin. Other devices may be added as additional tools or sections based on user preference during the Scope phase.

---

## D. Generated File Manifest

Create this directory structure for a new plugin package:

```
packages/<pkg>/
    CMakeLists.txt
    manifest.json.cmakein
    emu-xml/
        emu_setup.json
    plugins/<plugin>/
        CMakeLists.txt
        include/<plugin>/
            <plugin>plugin.h
            <tool>.h
            <helper>.h
            <plugin>_api.h
            scopy-<plugin>_config.h.cmakein
        src/
            <plugin>plugin.cpp
            <tool>.cpp
            <helper>.cpp
            <plugin>_api.cpp
        resources/
            resources.qrc
        test/
            CMakeLists.txt
            tst_pluginloader.cpp
```

Notes:
- The `scopy-<plugin>_export.h` file is auto-generated by CMake's `generate_export_header()` — do NOT create it manually.
- The tool class may be placed in a subdirectory (e.g., `src/<tool>/` and `include/<plugin>/<tool>/`) if the user prefers, following the ad936x pattern.

---

## E. Code Templates

### E.1 Copyright Header

Use this on ALL generated files (adjust comment style for CMake `#` vs C++ `/*`):

```cpp
/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */
```

### E.2 Plugin Header (`<plugin>plugin.h`)

```cpp
#ifndef {{PLUGIN_UPPER}}_PLUGIN_H
#define {{PLUGIN_UPPER}}_PLUGIN_H

#define SCOPY_PLUGIN_NAME {{PluginClass}}

#include "scopy-{{plugin}}_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::{{namespace}} {

class {{API_CLASS}};

class SCOPY_{{PLUGIN_UPPER}}_EXPORT {{PluginClass}} : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

	friend class {{API_CLASS}};

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;
	QString displayName() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	void initApi();

	{{API_CLASS}} *m_api = nullptr;
	IIOWidgetGroup *m_widgetGroup = nullptr;
};
} // namespace scopy::{{namespace}}
#endif // {{PLUGIN_UPPER}}_PLUGIN_H
```

### E.3 Plugin Source (`<plugin>plugin.cpp`)

```cpp
#include "{{plugin}}plugin.h"
#include "{{plugin}}_api.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-{{plugin}}_config.h"
#include <iioutil/connectionprovider.h>
#include <pluginbase/scopyjs.h>
#include <iio-widgets/iiowidgetgroup.h>

#include "{{tool_include}}"

Q_LOGGING_CATEGORY(CAT_{{PLUGIN_UPPER}}PLUGIN, "{{PluginClass}}")
using namespace scopy::{{namespace}};

bool {{PluginClass}}::compatible(QString m_param, QString category)
{
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		return false;
	}

	int device_count = iio_context_get_devices_count(conn->context());
	for(int i = 0; i < device_count; ++i) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		const char *dev_name = iio_device_get_name(dev);
		if(dev_name && QString(dev_name) == "{{primary_device_name}}") {
			ret = true;
			break;
		}
	}

	ConnectionProvider::close(m_param);
	return ret;
}

bool {{PluginClass}}::loadPage() { return false; }

bool {{PluginClass}}::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("{{DISPLAY_NAME}}");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void {{PluginClass}}::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("{{tool_id}}", "{{DISPLAY_NAME}}",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void {{PluginClass}}::unload() {}

QString {{PluginClass}}::description() { return "Plugin for {{DISPLAY_NAME}}"; }

QString {{PluginClass}}::displayName() { return {{PLUGIN_UPPER}}_PLUGIN_DISPLAY_NAME; }

bool {{PluginClass}}::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	{{ToolClass}} *tool = new {{ToolClass}}(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	initApi();
	return true;
}

bool {{PluginClass}}::onDisconnect()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}

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

	if(m_widgetGroup) {
		delete m_widgetGroup;
		m_widgetGroup = nullptr;
	}

	ConnectionProvider::close(m_param);
	return true;
}

void {{PluginClass}}::initApi()
{
	m_api = new {{API_CLASS}}(this);
	m_api->setObjectName("{{js_name}}");
	ScopyJS::GetInstance()->registerApi(m_api);
}

void {{PluginClass}}::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}
```

### E.4 Tool Header (`<tool>.h`)

```cpp
#ifndef {{TOOL_UPPER}}_H
#define {{TOOL_UPPER}}_H

#include "scopy-{{plugin}}_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>
#include <animatedrefreshbtn.h>
#include <{{helper_include}}>

namespace scopy {
class IIOWidgetGroup;
namespace {{namespace}} {

class SCOPY_{{PLUGIN_UPPER}}_EXPORT {{ToolClass}} : public QWidget
{
	Q_OBJECT
public:
	{{ToolClass}}(iio_context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~{{ToolClass}}();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_controlsWidget;
	AnimatedRefreshBtn *m_refreshButton;

	{{HelperClass}} *m_helper;
};
} // namespace {{namespace}}
} // namespace scopy
#endif // {{TOOL_UPPER}}_H
```

### E.5 Tool Source (`<tool>.cpp`)

```cpp
#include "{{tool_include_path}}"

#include <QLabel>
#include <QScrollArea>
#include <toolbuttons.h>
#include <style.h>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_{{TOOL_UPPER}}, "{{ToolClass}}");

using namespace scopy;
using namespace {{namespace}};

{{ToolClass}}::{{ToolClass}}(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });

		watcher->setFuture(future);
	});

	m_controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(m_controlsWidget);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);
	m_controlsWidget->setLayout(controlsLayout);

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlWidgetLayout->setMargin(0);
	controlWidgetLayout->setContentsMargins(0, 0, 0, 0);
	controlsWidget->setLayout(controlWidgetLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	if(m_ctx != nullptr) {
		// Find the primary device
		iio_device *device = iio_context_find_device(m_ctx, "{{primary_device_name}}");

		m_helper = new {{HelperClass}}(m_group);
		connect(this, &{{ToolClass}}::readRequested, m_helper, &{{HelperClass}}::readRequested);

		// Generate widget sections
		// {{GENERATE_SECTIONS_HERE}}

		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	m_tool->addWidgetToCentralContainerHelper(m_controlsWidget);
}

{{ToolClass}}::~{{ToolClass}}() {}
```

### E.6 Helper Header (`<helper>.h`)

```cpp
#ifndef {{HELPER_UPPER}}_H
#define {{HELPER_UPPER}}_H

#include "scopy-{{plugin}}_export.h"
#include <QWidget>
#include <iio-widgets/iiowidgetbuilder.h>

namespace scopy {
class IIOWidgetGroup;
namespace {{namespace}} {

class SCOPY_{{PLUGIN_UPPER}}_EXPORT {{HelperClass}} : public QWidget
{
	Q_OBJECT
public:
	{{HelperClass}}(IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);

	// Device-level settings section
	QWidget *generateGlobalSettingsWidget(iio_device *dev, QString title, QWidget *parent);

	// Per-direction chain sections
	// Add generate*Widget methods based on the device's channels

Q_SIGNALS:
	void readRequested();

private:
	IIOWidgetGroup *m_group = nullptr;
};
} // namespace {{namespace}}
} // namespace scopy
#endif // {{HELPER_UPPER}}_H
```

### E.7 Helper Source (`<helper>.cpp`)

The helper creates the actual IIOWidget instances. Each `generate*Widget` method follows this pattern:

```cpp
#include "{{helper}}.h"
#include <style.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_{{HELPER_UPPER}}, "{{HelperClass}}");

using namespace scopy;
using namespace {{namespace}};

{{HelperClass}}::{{HelperClass}}(IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_group(group)
{}

QWidget *{{HelperClass}}::generateGlobalSettingsWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	QHBoxLayout *hlayout = new QHBoxLayout();

	// === Device-level attributes ===
	// For each device-level attribute, create an IIOWidgetBuilder chain.

	// EXAMPLE: ComboUi with _available
	// IIOWidget *ensmMode = IIOWidgetBuilder(widget)
	// 	.device(dev)
	// 	.attribute("ensm_mode")
	// 	.optionsAttribute("ensm_mode_available")
	// 	.title("ENSM Mode")
	// 	.uiStrategy(IIOWidgetBuilder::ComboUi)
	// 	.group(m_group)
	// 	.buildSingle();
	// hlayout->addWidget(ensmMode);
	// connect(this, &{{HelperClass}}::readRequested, ensmMode, &IIOWidget::readAsync);

	layout->addLayout(hlayout);
	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}
```

### E.8 IIOWidgetBuilder Patterns

Use these exact patterns when creating widgets:

#### ComboUi (attribute with `_available` as discrete values)
```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
	.channel(chn)       // or .device(dev) for device-level
	.attribute("attr_name")
	.optionsAttribute("attr_name_available")
	.title("Display Title")
	.uiStrategy(IIOWidgetBuilder::ComboUi)
	.group(m_group)
	.buildSingle();
layout->addWidget(widget);
connect(this, &{{HelperClass}}::readRequested, widget, &IIOWidget::readAsync);
```

#### RangeUi (attribute with `_available` as `[min step max]`)
```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
	.channel(chn)
	.attribute("attr_name")
	.optionsAttribute("attr_name_available")
	.title("Display Title(Unit)")
	.uiStrategy(IIOWidgetBuilder::RangeUi)
	.group(m_group)
	.buildSingle();
// Add unit conversions if needed (see Section B)
layout->addWidget(widget);
connect(this, &{{HelperClass}}::readRequested, widget, &IIOWidget::readAsync);
```

#### CheckBoxUi (boolean `_en` attribute)
```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
	.channel(chn)
	.attribute("attr_name_en")
	.uiStrategy(IIOWidgetBuilder::CheckBoxUi)
	.title("Display Title")
	.group(m_group)
	.buildSingle();
widget->showProgressBar(false);
layout->addWidget(widget);
connect(this, &{{HelperClass}}::readRequested, widget, &IIOWidget::readAsync);
```

#### EditableUi (default, no `_available`)
```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
	.channel(chn)       // or .device(dev)
	.attribute("attr_name")
	.title("Display Title")
	.group(m_group)
	.buildSingle();
layout->addWidget(widget);
connect(this, &{{HelperClass}}::readRequested, widget, &IIOWidget::readAsync);
```

#### Read-only EditableUi (rssi, path_rates, etc.)
```cpp
IIOWidget *widget = IIOWidgetBuilder(parent)
	.device(dev)
	.attribute("rx_path_rates")
	.title("RX Path Rates")
	.group(m_group)
	.buildSingle();
layout->addWidget(widget);
widget->setEnabled(false);
connect(this, &{{HelperClass}}::readRequested, widget, &IIOWidget::readAsync);
```

### E.9 API Header (`<plugin>_api.h`)

```cpp
#ifndef {{PLUGIN_UPPER}}_API_H
#define {{PLUGIN_UPPER}}_API_H

#include "scopy-{{plugin}}_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::{{namespace}} {

class {{PluginClass}};

class SCOPY_{{PLUGIN_UPPER}}_EXPORT {{API_CLASS}} : public ApiObject
{
	Q_OBJECT
public:
	explicit {{API_CLASS}}({{PluginClass}} *plugin);
	~{{API_CLASS}}();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// === Named getters/setters for each attribute ===
	// Device-level attributes
	// Q_INVOKABLE QString getAttrName();
	// Q_INVOKABLE void setAttrName(const QString &value);

	// RX chain attributes
	// Q_INVOKABLE QString getRxAttrName();
	// Q_INVOKABLE void setRxAttrName(const QString &value);

	// TX chain attributes
	// Q_INVOKABLE QString getTxAttrName();
	// Q_INVOKABLE void setTxAttrName(const QString &value);

	// Per-channel methods (when multiple channels)
	// Q_INVOKABLE QString getRxHardwareGain(int channel);
	// Q_INVOKABLE void setRxHardwareGain(int channel, const QString &value);

	// Generic widget access (always include these)
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	{{PluginClass}} *m_plugin;
};

} // namespace scopy::{{namespace}}

#endif // {{PLUGIN_UPPER}}_API_H
```

### E.10 API Source (`<plugin>_api.cpp`)

```cpp
#include "{{plugin}}_api.h"
#include "{{plugin}}plugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_{{PLUGIN_UPPER}}_API, "{{API_CLASS}}")

using namespace scopy::{{namespace}};

{{API_CLASS}}::{{API_CLASS}}({{PluginClass}} *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

{{API_CLASS}}::~{{API_CLASS}}() {}

// --- Private helpers ---

QString {{API_CLASS}}::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_{{PLUGIN_UPPER}}_API) << "Widget group not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_{{PLUGIN_UPPER}}_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void {{API_CLASS}}::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_{{PLUGIN_UPPER}}_API) << "Widget group not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_{{PLUGIN_UPPER}}_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList {{API_CLASS}}::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// === Named getters/setters ===

// Device-level attribute example:
// QString {{API_CLASS}}::getEnsmMode() { return readFromWidget("{{device_name}}/ensm_mode"); }
// void {{API_CLASS}}::setEnsmMode(const QString &mode) { writeToWidget("{{device_name}}/ensm_mode", mode); }

// Channel-level attribute example:
// QString {{API_CLASS}}::getRxRfBandwidth() { return readFromWidget("{{device_name}}/voltage0_in/rf_bandwidth"); }
// void {{API_CLASS}}::setRxRfBandwidth(const QString &value) { writeToWidget("{{device_name}}/voltage0_in/rf_bandwidth", value); }

// Per-channel attribute example:
// QString {{API_CLASS}}::getRxHardwareGain(int channel)
// {
// 	QString chn = (channel == 0) ? "voltage0" : "voltage1";
// 	return readFromWidget("{{device_name}}/" + chn + "_in/hardwaregain");
// }

// Read-only attribute (no setter):
// QString {{API_CLASS}}::getRxPathRates() { return readFromWidget("{{device_name}}/rx_path_rates"); }

// --- Generic widget access ---

QStringList {{API_CLASS}}::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString {{API_CLASS}}::readWidget(const QString &key) { return readFromWidget(key); }

void {{API_CLASS}}::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void {{API_CLASS}}::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_{{plugin}}_api.cpp"
```

### E.11 Package CMakeLists.txt

```cmake
#
# Copyright (c) 2025 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE {{plugin}})
set(CURRENT_PKG_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PACKAGE_NAME ${SCOPY_MODULE})

set(PACKAGE_DISPLAY_NAME "{{DISPLAY_NAME}}")
set(PACKAGE_DESCRIPTION "Plugin for {{DISPLAY_NAME}}")

project(scopy-package-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

configure_file(manifest.json.cmakein ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/MANIFEST.json @ONLY)
include_emu_xml(${CMAKE_CURRENT_SOURCE_DIR}/emu-xml ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/emu-xml)
message(STATUS "Including plugins for ${SCOPY_MODULE}")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
	add_plugins(${CMAKE_CURRENT_SOURCE_DIR}/plugins ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/plugins)
	install_plugins(
		${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/plugins
		${SCOPY_PACKAGE_INSTALL_PATH}/${SCOPY_MODULE}/plugins "scopy"
	)
endif()
install_pkg(${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE} ${SCOPY_PACKAGE_INSTALL_PATH}/${SCOPY_MODULE})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(INSTALLER_DESCRIPTION "${PACKAGE_DISPLAY_NAME} - ${PACKAGE_DESCRIPTION}")
	configureinstallersettings(${SCOPY_MODULE} ${INSTALLER_DESCRIPTION} FALSE)
endif()
```

### E.12 Plugin CMakeLists.txt

```cmake
#
# Copyright (c) 2025 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE {{plugin}})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_NAME {{PluginClassName}})
set(PLUGIN_DISPLAY_NAME {{DISPLAY_NAME}})
set(PLUGIN_DESCRIPTION "Plugin for {{DISPLAY_NAME}}")

include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

file(
	GLOB
	SRC_LIST
	src/*.cpp
	src/*.cc
)
file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
)
file(GLOB UI_LIST ui/*.ui)

set(ENABLE_TESTING ON)
if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS REQUIRED Widgets Core)

qt_add_resources(PROJECT_RESOURCES resources/resources.qrc)
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})

target_include_directories(${PROJECT_NAME} PUBLIC scopy-pluginbase scopy-gui scopy-pkg-manager)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt::Widgets
	       Qt::Core
	       scopy-pluginbase
	       scopy-gui
	       scopy-iioutil
	       scopy-iio-widgets
	       scopy-pkg-manager
)

set({{PLUGIN_UPPER}}_TARGET_NAME ${PROJECT_NAME} PARENT_SCOPE)
```

### E.13 Config Header (`scopy-<plugin>_config.h.cmakein`)

```cpp
#ifndef SCOPY_{{PLUGIN_UPPER}}_CONFIG_H_CMAKEIN
#define SCOPY_{{PLUGIN_UPPER}}_CONFIG_H_CMAKEIN

#define {{PLUGIN_UPPER}}_PLUGIN_NAME "@PLUGIN_NAME@"
#define {{PLUGIN_UPPER}}_PLUGIN_DISPLAY_NAME "@PLUGIN_DISPLAY_NAME@"
#define {{PLUGIN_UPPER}}_PLUGIN_SCOPY_MODULE "@SCOPY_MODULE@"
#define {{PLUGIN_UPPER}}_PLUGIN_DESCRIPTION "@PLUGIN_DESCRIPTION@"

#cmakedefine ENABLE_SCOPYJS

#endif // SCOPY_{{PLUGIN_UPPER}}_CONFIG_H_CMAKEIN
```

### E.14 manifest.json.cmakein

```json
{
    "id": "{{plugin}}",
    "title": "{{DISPLAY_NAME}}",
    "version": "@PROJECT_VERSION@",
    "description": "This package includes a plugin that provides support for {{DISPLAY_NAME}}.",
    "license": "LGPL",
    "author": "Analog Devices Inc.",
    "download_link": "",
    "zip_checksum": "",
    "scopy_compatibility": ["@CMAKE_PROJECT_VERSION@"],
    "category": ["iio", "plugin", "base-pkg"]
}
```

### E.15 resources.qrc

```xml
<RCC>
    <qresource prefix="/{{plugin}}">
    </qresource>
</RCC>
```

### E.16 Test CMakeLists.txt

```cmake
#
# Copyright (c) 2025 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.5)

include(ScopyTest)

setup_scopy_tests(pluginloader)
```

### E.17 Test Source (`tst_pluginloader.cpp`)

```cpp
/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "qpluginloader.h"

#include <QList>
#include <QTest>

#include <pluginbase/plugin.h>

using namespace scopy;

class TST_{{PluginClass}} : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void fileExists();
	void isLibrary();
	void loaded();
	void className();
	void instanceNotNull();
	void multipleInstances();
	void qobjectcast_to_plugin();
	void clone();
	void name();
	void metadata();
};

#define PLUGIN_LOCATION "../.."
#define FILENAME PLUGIN_LOCATION "/libscopy-{{plugin}}.so"

void TST_{{PluginClass}}::fileExists()
{
	QFile f(FILENAME);
	bool ret;
	ret = f.open(QIODevice::ReadOnly);
	if(ret)
		f.close();
	QVERIFY(ret);
}

void TST_{{PluginClass}}::isLibrary() { QVERIFY(QLibrary::isLibrary(FILENAME)); }

void TST_{{PluginClass}}::className()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.metaData().value("className") == "{{PluginClass}}");
}

void TST_{{PluginClass}}::loaded()
{
	QPluginLoader qp(FILENAME, this);
	qp.load();
	QVERIFY(qp.isLoaded());
}

void TST_{{PluginClass}}::instanceNotNull()
{
	QPluginLoader qp(FILENAME, this);
	QVERIFY(qp.instance() != nullptr);
}

void TST_{{PluginClass}}::multipleInstances()
{
	QPluginLoader qp1(FILENAME, this);
	QPluginLoader qp2(FILENAME, this);

	QVERIFY(qp1.instance() == qp2.instance());
}

void TST_{{PluginClass}}::qobjectcast_to_plugin()
{
	QPluginLoader qp(FILENAME, this);
	auto instance = qobject_cast<Plugin *>(qp.instance());
	QVERIFY(instance != nullptr);
}

void TST_{{PluginClass}}::clone()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone();
	QVERIFY(p1 != nullptr);
	p2 = original->clone();
	QVERIFY(p2 != nullptr);
	QVERIFY(p1 != p2);
}

void TST_{{PluginClass}}::name()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	p1 = original->clone();
	qDebug() << p1->name();
}

void TST_{{PluginClass}}::metadata()
{
	QPluginLoader qp(FILENAME, this);

	Plugin *p1 = nullptr, *p2 = nullptr;
	auto original = qobject_cast<Plugin *>(qp.instance());
	original->initMetadata();
	p1 = original->clone();
	qDebug() << p1->metadata();
	QVERIFY(!p1->metadata().isEmpty());
}

QTEST_MAIN(TST_{{PluginClass}})

#include "tst_pluginloader.moc"
```

### E.18 EMU-XML (`emu_setup.json`)

```json
[
	{
		"device": "{{plugin}}",
		"xml_path": "{{device_name}}.xml",
		"uri": "ip:127.0.0.1"
	}
]
```

Note: The actual XML file for emulation is optional and complex to generate from iio_info. If needed, it should mirror the IIO device tree structure. You may skip EMU-XML generation and note it as a TODO for the user.

---

## F. IIOWidgetGroup Key Format

The `IIOWidgetGroup::generateKey()` method (from `iio-widgets/src/iiowidgetgroup.cpp`) produces keys as follows:

| Scope | Key Format | Example |
|-------|------------|---------|
| Channel attribute | `"device-name/channelId_dir/attribute"` | `"ad9361-phy/voltage0_in/rf_bandwidth"` |
| Device attribute | `"device-name/attribute"` | `"ad9361-phy/ensm_mode"` |
| Context attribute | `"context/attribute"` | `"context/hw_model"` |

Where:
- `device-name` = `iio_device_get_name(dev)` (falls back to `iio_device_get_id(dev)` if name is null)
- `channelId` = `iio_channel_get_id(channel)` (e.g., `voltage0`, `altvoltage1`)
- `dir` = `_in` for input channels, `_out` for output channels
- `attribute` = the attribute name (e.g., `rf_bandwidth`, `frequency`)

**Important**: The key uses a single `/` between device name and attribute for device-level attributes (NOT `//`). This differs from the documentation in `iiowidgetgroup_integration.md` which shows `//` — the actual code in `iiowidgetgroup.cpp` uses a single `/`.

---

## G. Naming Conventions

| Item | Convention | Example for "ad9361-phy" |
|------|-----------|--------------------------|
| Package directory | lowercase, no hyphens preferred | `packages/ad936x/` |
| Plugin module (SCOPY_MODULE) | lowercase | `ad936x` |
| Plugin class | PascalCase + Plugin | `Ad936xPlugin` |
| Tool class | UPPER or PascalCase | `AD936X` |
| Helper class | PascalCase + Helper | `AD936xHelper` |
| API class | UPPER + _API | `AD936X_API` |
| Namespace | `scopy::lowercase` | `scopy::ad936x` |
| Export macro | `SCOPY_UPPER_EXPORT` | `SCOPY_AD936X_EXPORT` |
| Config defines | `UPPER_PLUGIN_*` | `PLUTO_PLUGIN_NAME` |
| Header guard | `UPPER_H` | `AD936XPLUGIN_H` |
| Member variables | `m_` prefix | `m_widgetGroup` |
| Logging category | `CAT_UPPER` | `CAT_AD936XPLUGIN` |
| JS object name | lowercase | `"ad936x"` |
| Tool menu entry ID | camelCase + Tool | `"ad963xTool"` |

### Deriving Names from Device Name

Given a device name from iio_info (e.g., `adrv9002-phy`):
1. **Module/package**: strip `-phy`, convert to lowercase alphanumeric → `adrv9002`
2. **Plugin class**: PascalCase + Plugin → `Adrv9002Plugin`
3. **Tool class**: uppercase device family → `ADRV9002`
4. **Helper class**: PascalCase + Helper → `Adrv9002Helper`
5. **API class**: uppercase + _API → `ADRV9002_API`
6. **Namespace**: `scopy::adrv9002`
7. **JS name**: lowercase → `"adrv9002"`

Always confirm these names with the user during the Scope phase.

---

## H. Widget Section Organization

Organize widgets into sections based on the device structure:

### For a PHY device with RX and TX channels:

1. **Global Settings** (device-level attributes)
   - `ensm_mode`, `calib_mode`, `trx_rate_governor`, `xo_correction`
   - Read-only: `rx_path_rates`, `tx_path_rates`

2. **RX Chain** (input voltage channels shared attributes)
   - `rf_bandwidth`, `sampling_frequency`, `rf_port_select`
   - Tracking enables: `quadrature_tracking_en`, `rf_dc_offset_tracking_en`, `bb_dc_offset_tracking_en`

3. **RX Per-Channel** (per input voltage channel)
   - `hardwaregain`, `gain_control_mode`, `rssi`

4. **TX Chain** (output voltage channels shared attributes)
   - `rf_bandwidth`, `sampling_frequency`, `rf_port_select`

5. **TX Per-Channel** (per output voltage channel)
   - `hardwaregain`, `rssi`

6. **LO Frequencies** (altvoltage channels)
   - RX LO (`altvoltage0`): `frequency`
   - TX LO (`altvoltage1`): `frequency`

### Section Widget Pattern

Each section follows this visual pattern:
```
┌─────────────────────────────────────────┐
│ Section Title (QLabel, menuBig style)   │
│                                         │
│ ┌─────────┐ ┌─────────┐ ┌─────────┐   │
│ │ Widget1  │ │ Widget2  │ │ Widget3  │   │
│ └─────────┘ └─────────┘ └─────────┘   │
│                                         │
│ ┌──────────────────────────────────────┐ │
│ │ Per-channel sub-section              │ │
│ │ ┌────────┐ ┌────────┐               │ │
│ │ │ Ch1    │ │ Ch2    │               │ │
│ │ └────────┘ └────────┘               │ │
│ └──────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

Style calls for section widgets:
```cpp
Style::setBackgroundColor(widget, json::theme::background_primary);
Style::setStyle(widget, style::properties::widget::border_interactive);
```

Style calls for section titles:
```cpp
Style::setStyle(titleLabel, style::properties::label::menuBig);
```

---

## I. Interactive Workflow Phases

### Phase 1: Parse

1. Read the iio_info file provided as the argument.
2. Extract all devices, channels, and attributes following Section A rules.
3. Present a structured summary:

```
## iio_info Summary

### Context
- hw_model: Analog Devices PlutoSDR Rev.C (Z7010-AD9361)
- N context attributes

### Devices (N total)

#### Device 0: ad9361-phy (Primary PHY)
- 11 channels: 2 RX voltage, 2 TX voltage, 2 LO, 1 temp, ...
- 19 device attributes (N skipped, N usable)
- 183 debug attributes
- Role: Primary PHY → Main tool

#### Device 1: xadc (System Monitor)
- 10 channels: voltage rails + temp
- Role: System monitor (optional)

... (for each device)
```

4. Ask: "Does this summary look correct? Any devices I should focus on or skip?"

### Phase 2: Scope

1. Propose plugin naming (following Section G):
   - Package directory name
   - Plugin class name
   - Tool class name(s)
   - Helper class name
   - API class name
   - Namespace
   - JS object name

2. Propose tool organization (following Section C):
   - Which devices map to which tools
   - Whether to create an Advanced tool for debug attributes

3. Present the file manifest (following Section D).

4. Ask: "Do you approve this structure? Any changes to names or organization?"

### Phase 3: Design

1. For the primary PHY device, present the complete attribute-to-widget mapping:

```
## Attribute → Widget Mapping

### Device Attributes (ad9361-phy)
| Attribute | Has _available | Strategy | Title | Notes |
|-----------|---------------|----------|-------|-------|
| ensm_mode | Yes (combo) | ComboUi | ENSM Mode | |
| calib_mode | Yes (combo) | ComboUi | Calibration Mode | |
| rx_path_rates | No | EditableUi | RX Path Rates | Read-only |
| filter_fir_config | - | SKIP | - | Complex config |
| ... | | | | |

### RX Channel Attributes (voltage0 input)
| Attribute | Has _available | Strategy | Title | Conversion | Notes |
|-----------|---------------|----------|-------|------------|-------|
| rf_bandwidth | Yes (range) | RangeUi | RF Bandwidth(MHz) | Hz→MHz | |
| sampling_frequency | Yes (range) | RangeUi | Sampling Rate(MSPS) | SPS→MSPS | |
| hardwaregain | Yes (range) | RangeUi | Hardware Gain(dB) | Strip " dB" | |
| rssi | No | EditableUi | RSSI(dB) | - | Read-only |
| quadrature_tracking_en | No | CheckBoxUi | Quadrature | - | |
| ... | | | | | |

### Skipped Attributes
- filter_fir_config: Complex multi-line config
- gain_table_config: Complex multi-line config
- direct_reg_access: Debug register access
- waiting_for_supplier: Internal kernel attribute
- All ERROR attributes
```

2. Ask: "Do you approve this mapping? Any attributes to add, remove, or change strategy?"

### Phase 4: Generate

1. Create all files in order:
   a. Package-level: `CMakeLists.txt`, `manifest.json.cmakein`, `emu-xml/emu_setup.json`
   b. Plugin-level: `CMakeLists.txt`, config header, plugin header+source
   c. Tool: tool header+source
   d. Helper: helper header+source (with all IIOWidgetBuilder chains)
   e. API: API header+source
   f. Test: test CMakeLists.txt, test source
   g. Resources: `resources.qrc`

2. Replace all `{{placeholders}}` with actual values derived from the iio_info.

3. For the helper source, generate the actual IIOWidgetBuilder chains for every non-skipped attribute.

### Phase 5: Validate

Run through this checklist:

- [ ] All IIOWidgetBuilder chains include `.group(m_group)` before `.buildSingle()`
- [ ] All IIOWidgets are connected to `readRequested` signal
- [ ] All channel attributes use correct direction (input/output matches iio_info)
- [ ] All `_available` attributes are paired with their base attribute via `.optionsAttribute()`
- [ ] No ERROR attributes have widgets
- [ ] No `filter_fir_config`, `gain_table_config`, `direct_reg_access`, `waiting_for_supplier`, or `label` attributes have widgets
- [ ] Unit conversions are applied for `frequency`, `rf_bandwidth`, `sampling_frequency`, `hardwaregain`
- [ ] Read-only attributes (`rssi`, `*_path_rates`) have `setEnabled(false)`
- [ ] CheckBox attributes (`*_en`) use `CheckBoxUi` and `showProgressBar(false)`
- [ ] API header declares all getter/setter methods matching the widgets
- [ ] API source implements all declared methods with correct widget keys
- [ ] API source ends with `#include "moc_<plugin>_api.cpp"`
- [ ] Plugin header has `friend class` for API
- [ ] Plugin source calls `initApi()` at end of `onConnect()`
- [ ] Plugin source unregisters and deletes API in `onDisconnect()` before tool cleanup
- [ ] CMakeLists.txt file globs match the file locations
- [ ] Config header defines match the plugin's macro usage
- [ ] Test file references correct class name and library filename
- [ ] All files have Scopy copyright header
- [ ] All files use tabs for indentation
- [ ] All member variables use `m_` prefix
- [ ] Namespace is consistent across all files

Report any issues found and fix them before finishing.

---

## J. API Getter/Setter Naming

Follow these rules for API method names (from `api_class_generation.md`):

| Widget Key Pattern | Getter | Setter |
|-------------------|--------|--------|
| `dev/attr` (device-level) | `getAttrCamelCase()` | `setAttrCamelCase(value)` |
| `dev/voltageN_in/attr` | `getRxAttrCamelCase()` | `setRxAttrCamelCase(value)` |
| `dev/voltageN_out/attr` | `getTxAttrCamelCase()` | `setTxAttrCamelCase(value)` |
| `dev/altvoltageN_out/frequency` | `getRxLoFrequency()` / `getTxLoFrequency()` | Corresponding setter |
| `dev/voltageN_in/attr` (multiple N) | `getRxAttr(int channel)` | `setRxAttr(int channel, value)` |
| `*_en` attribute | `isAttrEnabled()` | `setAttrEnabled(value)` |
| Read-only (`rssi`, `*_path_rates`) | `getRxRssi()` or `getRxRssi(int channel)` | NO SETTER |

CamelCase conversion for attribute names:
- `rf_bandwidth` → `RfBandwidth`
- `sampling_frequency` → `SamplingFrequency`
- `hardwaregain` → `HardwareGain`
- `ensm_mode` → `EnsmMode`
- `quadrature_tracking_en` → `QuadratureTrackingEnabled`

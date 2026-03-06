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

#include "adrv9009plugin.h"
#include "adrv9009plugin_api.h"

#include <QLoggingCategory>
#include <QLabel>
#include <iio.h>
#include <adrv9009.h>
#include <adrv9009advanced.h>
#include <style.h>
#include <gui/deviceiconbuilder.h>

#include <style.h>
#include <gui/deviceiconbuilder.h>
#include <iioutil/connectionprovider.h>
#include <pluginbase/scopyjs.h>
#include <iio-widgets/iiowidgetgroup.h>

Q_LOGGING_CATEGORY(CAT_ADRV9009PLUGIN, "Adrv9009Plugin")
using namespace scopy::adrv9009;

bool Adrv9009Plugin::compatible(QString m_param, QString category)
{
	// Check for adrv9009-phy device family
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning(CAT_ADRV9009PLUGIN) << "Failed to open connection";
		return false;
	}

	// Look for ADRV9009 PHY device
	iio_device *device = iio_context_find_device(conn->context(), "adrv9009-phy");
	bool compatible = (device != nullptr);

	if(compatible) {
		qDebug(CAT_ADRV9009PLUGIN) << "ADRV9009 device found, plugin is compatible";
	} else {
		qDebug(CAT_ADRV9009PLUGIN) << "ADRV9009 device not found, plugin not compatible";
	}

	cp->close(m_param);
	return compatible;
}

bool Adrv9009Plugin::loadPage()
{
	// Here you must write the code for the plugin info page
	// Below is an example for an iio device
	/*m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ContextProvider::GetInstance();
	struct iio_context *context = cp->open(m_param);
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_ADRV9009PLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();
	return true;
	*/
	return false;
}

bool Adrv9009Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("ADRV9009");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Adrv9009Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("adrv9009tool", "ADRV9009",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ADRV9009 Advanced", "ADRV9009 Advanced",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Adrv9009Plugin::unload()
{ /*delete m_infoPage;*/
}

QString Adrv9009Plugin::description() { return "ADRV9009 RF Transceiver control and configuration plugin"; }

QString Adrv9009Plugin::generateAdvancedToolName(const char *deviceName)
{
	// Extract suffix (e.g., "adrv9009-phy-b" -> "ADRV9009 Advanced-B")
	QString suffix = QString(deviceName).mid(12); // Remove "adrv9009-phy"
	if(!suffix.isEmpty() && suffix.startsWith("-")) {
		suffix = suffix.mid(1).toUpper(); // Remove "-" and uppercase
		return QString("ADRV9009 Advanced-%1").arg(suffix);
	} else {
		return "ADRV9009 Advanced";
	}
}

void Adrv9009Plugin::createAdditionalAdvancedTool(iio_device *device, const char *deviceName)
{
	// Generate the tool name
	QString advancedToolName = generateAdvancedToolName(deviceName);

	// Create advanced tool menu entry (using same string for ID and name)
	ToolMenuEntry *advancedEntry = SCOPY_NEW_TOOLMENUENTRY(
		advancedToolName, advancedToolName,
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/gear_wheel.svg");

	// Add to tool list first
	m_toolList.append(advancedEntry);
	advancedEntry->setEnabled(true);
	advancedEntry->setRunBtnVisible(true);

	// Emit toolListChanged to establish signal connections
	Q_EMIT toolListChanged();

	// Then create widget and set tool
	Adrv9009Advanced *adrv9009Advanced = new Adrv9009Advanced(device, m_widgetGroup);
	advancedEntry->setTool(adrv9009Advanced);

	qDebug(CAT_ADRV9009PLUGIN) << "Created" << advancedToolName << "for device:" << deviceName;
}

bool Adrv9009Plugin::onConnect()
{
	// Open connection and pass context to tool
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning(CAT_ADRV9009PLUGIN) << "Failed to open connection for tool initialization";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	// Create basic ADRV9009 tool with IIO context
	Adrv9009 *adrv9009 = new Adrv9009(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(adrv9009);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	// Iterate through devices to set up ADRV9009 Advanced tools
	iio_context *ctx = conn->context();
	unsigned int deviceCount = iio_context_get_devices_count(ctx);
	bool first = true;

	for(unsigned int i = 0; i < deviceCount; i++) {
		iio_device *device = iio_context_get_device(ctx, i);
		const char *deviceName = iio_device_get_name(device);

		if(deviceName && QString(deviceName).startsWith("adrv9009-phy")) {
			if(first) {
				// Set up existing "ADRV9009 Advanced" tool (m_toolList[1])
				Adrv9009Advanced *adrv9009Advanced = new Adrv9009Advanced(device, m_widgetGroup);
				m_toolList[1]->setTool(adrv9009Advanced);
				m_toolList[1]->setEnabled(true);
				m_toolList[1]->setRunBtnVisible(true);

				qDebug(CAT_ADRV9009PLUGIN)
					<< "Set up ADRV9009 Advanced for first device:" << deviceName;
				first = false;
			} else {
				createAdditionalAdvancedTool(device, deviceName);
			}
		}
	}

	qDebug(CAT_ADRV9009PLUGIN) << "ADRV9009 plugin connected successfully with" << (m_toolList.size() - 1)
				   << "advanced tools";

	// Emit signal to notify system about tool list changes
	Q_EMIT toolListChanged();

	initApi();
	return true;
}

bool Adrv9009Plugin::onDisconnect()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}

	// Clean up tools and close connection
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

	// Close connection
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	cp->close(m_param);

	qDebug(CAT_ADRV9009PLUGIN) << "ADRV9009 plugin disconnected successfully";
	return true;
}

void Adrv9009Plugin::initApi()
{
	m_api = new Adrv9009Plugin_API(this);
	m_api->setObjectName("adrv9009");
	ScopyJS::GetInstance()->registerApi(m_api);
}

void Adrv9009Plugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}

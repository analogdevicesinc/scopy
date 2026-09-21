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

#include "ad9084plugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include <style.h>
#include <gui/deviceiconbuilder.h>

#include "ad9084.h"
#include "ad9084_api.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/scopyjs.h>

#include <component/controller.h>
#include <component/context.h>
#include <component/device.h>

Q_LOGGING_CATEGORY(CAT_AD9084PLUGIN, "AD9084Plugin")
using namespace scopy::ad9084;

bool AD9084Plugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = false;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		qDebug(CAT_AD9084PLUGIN) << "The context is not compatible with the AD9084 Plugin!";
		return ret;
	}

	component::Device *apolloDevice =
		ctx->findChild<component::Device *>("axi-ad9084-rx-hpc", Qt::FindDirectChildrenOnly);
	if(!apolloDevice) {
		apolloDevice = ctx->findChild<component::Device *>("axi-ad9088-rx-hpc", Qt::FindDirectChildrenOnly);
	}

	if(apolloDevice) {
		ret = true;
	}
	return ret;
}

bool AD9084Plugin::loadPage() { return false; }

bool AD9084Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("APOLLO");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
	return true;
}

void AD9084Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad9084", "AD9084",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_network_analyzer.svg"));
}

void AD9084Plugin::unload()
{ /*delete m_infoPage;*/
}

QString AD9084Plugin::description() { return "Tool for Apollo MxFE interaction."; }

bool AD9084Plugin::onConnect()
{
	int deviceIdx = 1;
	m_context = component::Controller::context(m_param);
	if(!m_context) {
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	component::Device *dev =
		m_context->findChild<component::Device *>("axi-ad9084-rx-hpc", Qt::FindDirectChildrenOnly);
	m_toolList.last()->setTool(new Ad9084(dev, m_widgetGroup));
	m_toolList.last()->setEnabled(true);
	m_toolList.last()->setRunBtnVisible(true);

	const auto devices = m_context->findChildren<component::Device *>(QString(), Qt::FindDirectChildrenOnly);
	for(component::Device *rxDev : devices) {
		bool newTool = false;
		QString toolName = "";
		if(!rxDev) {
			continue;
		}
		if(rxDev == dev) {
			continue;
		}

		QString name = rxDev->name();
		if(name.contains("axi-ad9084-rx")) {
			newTool = true;
			toolName = "AD9084";
		}
		if(name.contains("axi-ad9088-rx")) {
			newTool = true;
			toolName = "AD9088";
		}

		if(newTool) {
			m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(
				"ad9084", toolName,
				":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
					"/icons/tool_network_analyzer.svg"));
			m_toolList.last()->setEnabled(true);
			m_toolList.last()->setRunBtnVisible(true);
			Q_EMIT toolListChanged();
			Ad9084 *ad9084 = new Ad9084(rxDev, m_widgetGroup);
			m_toolList.last()->setTool(ad9084);
			deviceIdx++;
		}
	}

	initApi();
	return true;
}

bool AD9084Plugin::onDisconnect()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}

	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			m_toolList.removeOne(tool);
			tool->setTool(nullptr);
			delete(w);
		}
	}
	loadToolList();
	Q_EMIT toolListChanged();

	if(m_widgetGroup) {
		delete m_widgetGroup;
		m_widgetGroup = nullptr;
	}

	m_context = {};
	return true;
}

void AD9084Plugin::initApi()
{
	m_api = new AD9084_API(this);
	m_api->setObjectName("ad9084");
	ScopyJS::GetInstance()->registerApi(m_api);
}

void AD9084Plugin::initMetadata()
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

#include "moc_ad9084plugin.cpp"

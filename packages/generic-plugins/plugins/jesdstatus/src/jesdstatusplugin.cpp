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

#include "jesdstatusplugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include <style.h>
#include "jesdstatus.h"
#include "component/controller.h"

#include <component/attribute.h>
#include <component/device.h>

Q_LOGGING_CATEGORY(CAT_JESDSTATUSPLUGIN, "JesdStatusPlugin")
using namespace scopy;
using namespace scopy::jesdstatus;

bool JesdStatusPlugin::compatible(QString m_param, QString category)
{
	bool ret = false;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		qDebug(CAT_JESDSTATUSPLUGIN) << "The context is not compatible with the JesdStatus Plugin!";
		return ret;
	}

	auto lst = scanCompatibleDevices(ctx.get());
	ret = lst.size();
	return ret;
}

QList<component::Device *> JesdStatusPlugin::scanCompatibleDevices(component::Context *ctx)
{
	QList<component::Device *> devList = {};
	for(component::Device *dev : ctx->findChildren<component::Device *>(Qt::FindDirectChildrenOnly)) {
		component::Attribute *statusAttr =
			dev->findChild<component::Attribute *>("status", Qt::FindDirectChildrenOnly);
		if(!statusAttr) {
			continue;
		}
		QString name = dev->name();
		QString id = dev->id();
		QString label = dev->label();
		if(name.contains("jesd") || id.contains("jesd") || label.contains("jesd")) {
			devList.push_back(dev);
		}
	}
	return devList;
}

bool JesdStatusPlugin::loadPage() { return false; }

bool JesdStatusPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void JesdStatusPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("jesdstatus", "JESD STATUS",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_voltmeter.svg"));
}

void JesdStatusPlugin::unload()
{ /*delete m_infoPage;*/
}

QString JesdStatusPlugin::description() { return "JESD status GUI tool for compatible devices"; }

bool JesdStatusPlugin::onConnect()
{
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}
	QList<component::Device *> devLst = scanCompatibleDevices(ctx.get());

	JesdStatus *jesdStatus = new JesdStatus(devLst);
	m_toolList[0]->setTool(jesdStatus);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	connect(m_toolList.last(), &ToolMenuEntry::runToggled, dynamic_cast<JesdStatus *>(jesdStatus),
		&JesdStatus::runToggled);
	connect(dynamic_cast<JesdStatus *>(jesdStatus), &JesdStatus::running, m_toolList[0],
		&ToolMenuEntry::setRunning);
	return true;
}

bool JesdStatusPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
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
	return true;
}

void JesdStatusPlugin::initMetadata()
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

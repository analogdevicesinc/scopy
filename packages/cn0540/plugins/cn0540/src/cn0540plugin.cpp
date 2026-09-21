/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "cn0540plugin.h"
#include "cn0540.h"
#include "cn0540_api.h"

#include <QLoggingCategory>

#include <pluginbase/scopyjs.h>
#include <style.h>

#include <component/controller.h>
#include <component/context.h>
#include <component/device.h>

Q_LOGGING_CATEGORY(CAT_CN0540PLUGIN, "CN0540Plugin")

using namespace scopy::cn0540;

bool CN0540Plugin::compatible(QString m_param, QString category)
{
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		qDebug(CAT_CN0540PLUGIN) << "No context available for CN0540";
		return false;
	}

	bool ret = ctx->findChild<component::Device *>("ad7768-1", Qt::FindDirectChildrenOnly) &&
		ctx->findChild<component::Device *>("ltc2606", Qt::FindDirectChildrenOnly) &&
		ctx->findChild<component::Device *>("one-bit-adc-dac", Qt::FindDirectChildrenOnly);

	if(ret)
		qDebug(CAT_CN0540PLUGIN) << "Found CN0540 devices";

	return ret;
}

bool CN0540Plugin::loadPage() { return false; }

bool CN0540Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void CN0540Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("cn0540tool", "CN0540",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_debugger.svg"));
}

void CN0540Plugin::unload() {}

QString CN0540Plugin::description()
{
	return "CN0540 precision measurement system plugin ported from iio-oscilloscope";
}

QString CN0540Plugin::displayName() { return "CN0540"; }

bool CN0540Plugin::onConnect()
{
	m_context = component::Controller::context(m_param);
	if(!m_context) {
		qWarning(CAT_CN0540PLUGIN) << "No context available for CN0540";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);
	CN0540 *tool = new CN0540(m_context.get(), m_widgetGroup);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

void CN0540Plugin::initApi()
{
	m_api = new CN0540_API(this);
	m_api->setObjectName("cn0540");
	ScopyJS::GetInstance()->registerApi(m_api);
}

bool CN0540Plugin::onDisconnect()
{
	ScopyJS::GetInstance()->unregisterApi(m_api);

	if(m_api != nullptr) {
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

	delete m_widgetGroup;
	m_widgetGroup = nullptr;

	m_context = {};
	return true;
}

void CN0540Plugin::initMetadata()
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

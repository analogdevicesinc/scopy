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
 *
 */

#include "fmcomms11plugin.h"
#include "fmcomms11.h"
#include "fmcomms11_api.h"
#include <pluginbase/scopyjs.h>

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-fmcomms11_config.h"
#include <iio-widgets/iiowidgetgroup.h>

#include <component/controller.h>
#include <component/context.h>
#include <component/device.h>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11PLUGIN, "Fmcomms11Plugin")
using namespace scopy::fmcomms11;

bool Fmcomms11Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_FMCOMMS11PLUGIN) << "Check FMCOMMS11 compatibility";
	component::ContextHandle ctx = component::Controller::context(m_param);

	if(!ctx) {
		qWarning(CAT_FMCOMMS11PLUGIN) << "No context available for FMCOMMS11";
		return false;
	}

	bool adc = ctx->findChild<component::Device *>("axi-ad9625-hpc", Qt::FindDirectChildrenOnly) != nullptr;
	bool dac = ctx->findChild<component::Device *>("axi-ad9162-hpc", Qt::FindDirectChildrenOnly) != nullptr;
	bool attn = ctx->findChild<component::Device *>("hmc1119", Qt::FindDirectChildrenOnly) != nullptr;
	bool vga = ctx->findChild<component::Device *>("adl5240", Qt::FindDirectChildrenOnly) != nullptr;

	return adc && dac && attn && vga;
}

bool Fmcomms11Plugin::loadPage() { return false; }

bool Fmcomms11Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("FMCOMMS11");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Fmcomms11Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("fmcomms11Tool", "FMCOMMS11",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_debugger.svg"));
}

void Fmcomms11Plugin::unload() {}

QString Fmcomms11Plugin::description() { return "This is a plugin for FMCOMMS11"; }

QString Fmcomms11Plugin::displayName() { return FMCOMMS11_PLUGIN_DISPLAY_NAME; }

bool Fmcomms11Plugin::onConnect()
{
	m_context = component::Controller::context(m_param);

	if(!m_context) {
		qWarning(CAT_FMCOMMS11PLUGIN) << "No context available for FMCOMMS11";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	FMCOMMS11 *tool = new FMCOMMS11(m_context.get(), m_widgetGroup);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

bool Fmcomms11Plugin::onDisconnect()
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

	m_context = {};
	return true;
}

void Fmcomms11Plugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["m2kplugin"]
	}
)plugin");
}

void Fmcomms11Plugin::initApi()
{
	m_api = new Fmcomms11_API(this);
	m_api->setObjectName("fmcomms11");
	ScopyJS::GetInstance()->registerApi(m_api);
}

#include "moc_fmcomms11plugin.cpp"

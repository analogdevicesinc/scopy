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

#include "cn0511plugin.h"
#include "cn0511.h"
#include "cn0511_api.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-cn0511_config.h"
#include <component/attribute.h>
#include <component/attributewriter.h>
#include <component/context.h>
#include <component/device.h>
#include <component/navigation.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/scopyjs.h>
#include <QCoro/QCoroTask>

Q_LOGGING_CATEGORY(CAT_CN0511PLUGIN, "Cn0511Plugin")
using namespace scopy::cn0511;

namespace {

// Blocking one-shot attribute write. onConnect() is a bool-returning override and
// cannot be a coroutine, so the task is driven to completion with QCoro::waitFor.
void writeAttr(scopy::component::Device *dev, const QString &name, const QString &value)
{
	scopy::component::Attribute *attr = scopy::component::attributeByName(dev, name);
	if(!attr || !attr->writeCapability()) {
		qWarning(CAT_CN0511PLUGIN) << "Attribute" << name << "is not writable";
		return;
	}

	auto response = QCoro::waitFor(attr->writeCapability()->writeAsync(value));
	if(!response) {
		qWarning(CAT_CN0511PLUGIN) << "Failed to write" << name << "Error:" << response.error().message;
	}
}

} // namespace

bool Cn0511Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_CN0511PLUGIN) << "Check CN0511 compatibility";

	component::ContextHandle ctx = component::Controller::context(m_param);

	if(!ctx) {
		qWarning(CAT_CN0511PLUGIN) << "No context available for CN0511";
		return false;
	}

	return ctx->findChild<component::Device *>("ad9166", Qt::FindDirectChildrenOnly) != nullptr &&
		ctx->findChild<component::Device *>("ad9166-amp", Qt::FindDirectChildrenOnly) != nullptr;
}

bool Cn0511Plugin::loadPage() { return false; }

bool Cn0511Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("CN0511");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Cn0511Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("cn0511Tool", "CN0511",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_debugger.svg"));
}

void Cn0511Plugin::unload() {}

QString Cn0511Plugin::description() { return CN0511_PLUGIN_DESCRIPTION; }

QString Cn0511Plugin::displayName() { return CN0511_PLUGIN_DISPLAY_NAME; }

bool Cn0511Plugin::onConnect()
{
	m_context = component::Controller::context(m_param);

	if(!m_context) {
		qWarning(CAT_CN0511PLUGIN) << "No context available for CN0511";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	// Initialize device settings as per original cn0511.c
	component::Device *dac = m_context->findChild<component::Device *>("ad9166", Qt::FindDirectChildrenOnly);
	if(dac) {
		writeAttr(dac, "fir85_enable", "1");
		writeAttr(dac, "sampling_frequency", QString::number(6000000000LL));
	}

	CN0511 *cn0511 = new CN0511(m_context.get(), m_widgetGroup);
	m_toolList[0]->setTool(cn0511);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

void Cn0511Plugin::initApi()
{
	m_api = new Cn0511_API(this);
	m_api->setObjectName("cn0511");
	ScopyJS::GetInstance()->registerApi(m_api);
}

bool Cn0511Plugin::onDisconnect()
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

void Cn0511Plugin::initMetadata()
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

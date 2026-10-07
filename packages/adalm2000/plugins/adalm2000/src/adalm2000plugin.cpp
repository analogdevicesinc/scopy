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

#include "adalm2000plugin.h"

#include "powersupply/adalm2000powersupplytool.h"
#include "voltmeter/adalm2000voltmetertool.h"
#include "m2kcalibration.h"
#include "m2kcontext.h"
#include "m2kcontextfactory.h"

#include <component/device.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <style.h>

#include <QLabel>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADALM2000PLUGIN, "Adalm2000Plugin")

using namespace scopy;
using namespace scopy::adalm2000;

void Adalm2000Plugin::init()
{
	component::Controller::GetInstance()->registerFactory(component::BackendKind::M2k,
							      std::make_shared<M2kContextFactory>());
}

bool Adalm2000Plugin::compatible(QString param, QString category)
{
	Q_UNUSED(category)
	component::ContextHandle ctx = component::Controller::context(param);
	if(!ctx) {
		return false;
	}
	return ctx->findChild<component::Device *>("m2k-adc", Qt::FindDirectChildrenOnly) &&
		ctx->findChild<component::Device *>("m2k-dac-a", Qt::FindDirectChildrenOnly) &&
		ctx->findChild<component::Device *>("m2k-dac-b", Qt::FindDirectChildrenOnly);
}

bool Adalm2000Plugin::loadPage() { return false; }

bool Adalm2000Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void Adalm2000Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("adalm2000_dmm", "Voltmeter",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_voltmeter.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("adalm2000_power", "Power Supply",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_power_supply.svg"));
}

void Adalm2000Plugin::unload() {}

QString Adalm2000Plugin::description() { return "ADALM2000 (libm2k backend)"; }

bool Adalm2000Plugin::onConnect()
{
	m_context = component::Controller::context(m_param);
	if(!m_context) {
		qWarning(CAT_ADALM2000PLUGIN) << "No context for" << m_param;
		return false;
	}

	if(!qobject_cast<M2kContext *>(m_context.get())) {
		qWarning(CAT_ADALM2000PLUGIN) << "Context for" << m_param << "is not an M2kContext:"
					      << "libm2k could not open this device";
		m_context = {};
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	auto *tool = new Adalm2000VoltmeterTool(m_toolList[0], m_context.get(), m_widgetGroup, m_param);
	m_toolList[0]->setTool(tool);

	auto *powerTool = new Adalm2000PowerSupplyTool(m_context.get(), m_widgetGroup);
	m_toolList[1]->setTool(powerTool);

	for(auto &entry : m_toolList) {
		entry->setEnabled(false);
	}
	calibrateAsync();
	return true;
}

QCoro::Task<void> Adalm2000Plugin::calibrateAsync()
{
	auto *m2kCtx = qobject_cast<M2kContext *>(m_context.get());
	if(!m2kCtx) {
		co_return;
	}

	M2kCalibration calibration(m2kCtx, this);
	m_calibrated = co_await calibration.run();

	for(auto &entry : m_toolList) {
		entry->setEnabled(true);
	}
	// Only the Voltmeter has a run button; the Power Supply's rails are enabled
	// individually from its settings menu.
	m_toolList[0]->setRunBtnVisible(true);

	if(!m_calibrated) {
		qWarning(CAT_ADALM2000PLUGIN) << "proceeding with uncalibrated readings";
	}
}

bool Adalm2000Plugin::onDisconnect()
{
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete w;
		}
	}

	if(m_widgetGroup) {
		delete m_widgetGroup;
		m_widgetGroup = nullptr;
	}

	m_context = {};
	return true;
}

void Adalm2000Plugin::initMetadata()
{
	loadMetadata(R"plugin(
	{
	   "disconnectDevOnConnectFailure":true,
	   "priority":100,
	   "category":[
	      "iio",
	      "m2k"
	   ],
	   "exclude":["*"]
	}
)plugin");
}

#include "moc_adalm2000plugin.cpp"

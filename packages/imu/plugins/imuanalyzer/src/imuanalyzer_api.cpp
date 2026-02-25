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

#include "imuanalyzer_api.h"
#include "imuanalyzer.h"
#include "imuanalyzerinterface.hpp"
#include <pluginbase/toolmenuentry.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_IMUANALYZER_API, "IMUAnalyzer_API")

using namespace scopy;

IMUAnalyzer_API::IMUAnalyzer_API(IMUAnalyzer *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

IMUAnalyzer_API::~IMUAnalyzer_API() {}

// --- Private helpers ---

IMUAnalyzerInterface *IMUAnalyzer_API::getIMUInterface()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_plugin->m_toolList, "imuanalyzer");
	if(tool && tool->tool()) {
		return dynamic_cast<IMUAnalyzerInterface *>(tool->tool());
	}
	return nullptr;
}

// --- Tool management ---

QStringList IMUAnalyzer_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Run control ---

bool IMUAnalyzer_API::isRunning()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? inst->m_runThread.load() : false;
}

void IMUAnalyzer_API::setRunning(bool running)
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_plugin->m_toolList, "imuanalyzer");
	if(tool) {
		tool->setRunning(running);
		Q_EMIT tool->runClicked(running);
	}
}

// --- Measure panel ---

bool IMUAnalyzer_API::isMeasureEnabled()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst && inst->m_measureBtn ? inst->m_measureBtn->isChecked() : false;
}

void IMUAnalyzer_API::setMeasureEnabled(bool enabled)
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	if(inst && inst->m_measureBtn) {
		inst->m_measureBtn->setChecked(enabled);
	}
}

// --- Sensor data ---

double IMUAnalyzer_API::getRotationX()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? static_cast<double>(inst->m_rot.dataX) : 0.0;
}

double IMUAnalyzer_API::getRotationY()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? static_cast<double>(inst->m_rot.dataY) : 0.0;
}

double IMUAnalyzer_API::getRotationZ()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? static_cast<double>(inst->m_rot.dataZ) : 0.0;
}

double IMUAnalyzer_API::getAccelerationX()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? static_cast<double>(inst->m_dist.dataX) : 0.0;
}

double IMUAnalyzer_API::getAccelerationY()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? static_cast<double>(inst->m_dist.dataY) : 0.0;
}

double IMUAnalyzer_API::getAccelerationZ()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	return inst ? static_cast<double>(inst->m_dist.dataZ) : 0.0;
}

// --- 3D view ---

void IMUAnalyzer_API::resetView()
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	if(inst && inst->m_sceneRender) {
		inst->m_sceneRender->resetView();
	}
}

// --- 2D view ---

void IMUAnalyzer_API::setDisplayPoints(const QString &mode)
{
	IMUAnalyzerInterface *inst = getIMUInterface();
	if(inst && inst->m_bubbleLevelRenderer) {
		inst->m_bubbleLevelRenderer->setDisplayPoints(mode);
	}
}

#include "moc_imuanalyzer_api.cpp"

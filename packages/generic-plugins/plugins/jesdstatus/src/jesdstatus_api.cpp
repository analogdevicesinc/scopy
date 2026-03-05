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

#include "jesdstatus_api.h"
#include "jesdstatusplugin.h"
#include "jesdstatus.h"
#include "jesdstatusview.h"
#include "jesdstatusparser.h"
#include <pluginbase/toolmenuentry.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_JESDSTATUS_API, "JesdStatus_API")

using namespace scopy::jesdstatus;

JesdStatus_API::JesdStatus_API(JesdStatusPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

JesdStatus_API::~JesdStatus_API() {}

// --- Private helpers ---

JesdStatus *JesdStatus_API::getJesdStatus()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_plugin->m_toolList, "jesdstatus");
	if(tool && tool->tool()) {
		return dynamic_cast<JesdStatus *>(tool->tool());
	}
	return nullptr;
}

JesdStatusView *JesdStatus_API::getCurrentView()
{
	JesdStatus *inst = getJesdStatus();
	if(!inst || !inst->m_deviceSelector || !inst->m_jesdDeviceStack) {
		return nullptr;
	}
	QString device = inst->m_deviceSelector->combo()->currentText();
	return dynamic_cast<JesdStatusView *>(inst->m_jesdDeviceStack->get(device));
}

JesdStatusParser *JesdStatus_API::getCurrentParser()
{
	JesdStatusView *view = getCurrentView();
	if(!view) {
		return nullptr;
	}
	return view->m_parser;
}

// --- Tool management ---

QStringList JesdStatus_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Run control ---

bool JesdStatus_API::isRunning()
{
	JesdStatus *inst = getJesdStatus();
	return inst && inst->m_timer ? inst->m_timer->isActive() : false;
}

void JesdStatus_API::setRunning(bool running)
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_plugin->m_toolList, "jesdstatus");
	if(tool) {
		tool->setRunning(running);
		Q_EMIT tool->runClicked(running);
	}
}

// --- Device selection ---

QStringList JesdStatus_API::getDevices()
{
	JesdStatus *inst = getJesdStatus();
	QStringList devices;
	if(inst && inst->m_deviceSelector) {
		for(int i = 0; i < inst->m_deviceSelector->combo()->count(); i++) {
			devices.append(inst->m_deviceSelector->combo()->itemText(i));
		}
	}
	return devices;
}

QString JesdStatus_API::getSelectedDevice()
{
	JesdStatus *inst = getJesdStatus();
	return inst && inst->m_deviceSelector ? inst->m_deviceSelector->combo()->currentText() : QString();
}

void JesdStatus_API::setSelectedDevice(const QString &device)
{
	JesdStatus *inst = getJesdStatus();
	if(inst && inst->m_deviceSelector) {
		int index = inst->m_deviceSelector->combo()->findText(device);
		if(index >= 0) {
			inst->m_deviceSelector->combo()->setCurrentIndex(index);
		}
	}
}

// --- Refresh ---

void JesdStatus_API::refresh()
{
	JesdStatus *inst = getJesdStatus();
	if(inst) {
		inst->poll();
	}
}

// --- Link status ---

QString JesdStatus_API::getLinkState()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getLinkState().first : QString();
}

QString JesdStatus_API::getLinkStatus()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getLinkStatus().first : QString();
}

QString JesdStatus_API::getMeasuredLinkClock()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getMeasuredLinkClock().first : QString();
}

QString JesdStatus_API::getReportedLinkClock()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getReportedLinkClock().first : QString();
}

QString JesdStatus_API::getMeasuredDeviceClock()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getMeasuredDeviceClock().first : QString();
}

QString JesdStatus_API::getReportedDeviceClock()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getReportedDeviceClock().first : QString();
}

QString JesdStatus_API::getDesiredDeviceClock()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getDesiredDeviceClock().first : QString();
}

QString JesdStatus_API::getLaneRate()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getLaneRate().first : QString();
}

QString JesdStatus_API::getLaneRateDiv()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getLaneRateDiv().first : QString();
}

QString JesdStatus_API::getLmfcRate()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getLmfcRate().first : QString();
}

QString JesdStatus_API::getSysrefCaptured()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getSysrefCaptured().first : QString();
}

QString JesdStatus_API::getSysrefAlignmentError()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getSysrefAlignmentError().first : QString();
}

QString JesdStatus_API::getSyncState()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getSyncState().first : QString();
}

// --- Lane status ---

int JesdStatus_API::getLaneCount()
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? static_cast<int>(parser->getLaneCount()) : 0;
}

QString JesdStatus_API::getEncoder()
{
	JesdStatusParser *parser = getCurrentParser();
	if(!parser) {
		return QString();
	}
	switch(parser->getEncoder()) {
	case JESD204_8B10B:
		return QStringLiteral("8B10B");
	case JESD204_64B66B:
		return QStringLiteral("64B66B");
	default:
		return QStringLiteral("UNKNOWN");
	}
}

QString JesdStatus_API::getLaneErrors(int lane)
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getErrors(static_cast<unsigned int>(lane)).first : QString();
}

QString JesdStatus_API::getLaneLatency(int lane)
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getLaneLatency(static_cast<unsigned int>(lane)).first : QString();
}

QString JesdStatus_API::getLaneCgsState(int lane)
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getCgsState(static_cast<unsigned int>(lane)).first : QString();
}

QString JesdStatus_API::getLaneInitFrameSync(int lane)
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getInitFrameSync(static_cast<unsigned int>(lane)).first : QString();
}

QString JesdStatus_API::getLaneInitLaneAlignSeq(int lane)
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getInitLaneAlignSeq(static_cast<unsigned int>(lane)).first : QString();
}

QString JesdStatus_API::getLaneExtMultiBlockAlignment(int lane)
{
	JesdStatusParser *parser = getCurrentParser();
	return parser ? parser->getExtMultiBlockAlignment(static_cast<unsigned int>(lane)).first : QString();
}

#include "moc_jesdstatus_api.cpp"

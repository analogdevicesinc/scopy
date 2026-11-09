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

#include "epromtest_api.h"
#include "epromtestplugin.h"

#include <QLoggingCategory>

#include <pluginbase/toolmenuentry.h>

Q_LOGGING_CATEGORY(CAT_EPROMTEST_API, "EpromTest_API")

using namespace scopy;

EpromTest_API::EpromTest_API(EpromTestPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

EpromTest_API::~EpromTest_API() {}

QStringList EpromTest_API::getTools()
{
	QStringList tools;
	if(!m_plugin) {
		return tools;
	}
	for(ToolMenuEntry *tool : std::as_const(m_plugin->m_toolList)) {
		tools.append(tool->name());
	}
	return tools;
}

bool EpromTest_API::sync()
{
	if(!m_plugin) {
		qWarning(CAT_EPROMTEST_API) << "Plugin not available";
		return false;
	}
	return m_plugin->sync();
}

bool EpromTest_API::configure()
{
	if(!m_plugin) {
		qWarning(CAT_EPROMTEST_API) << "Plugin not available";
		return false;
	}
	return m_plugin->configure();
}

QString EpromTest_API::getStructure()
{
	if(!m_plugin) {
		qWarning(CAT_EPROMTEST_API) << "Plugin not available";
		return QString();
	}
	return m_plugin->structure();
}

void EpromTest_API::switchDevice()
{
	if(!m_plugin) {
		qWarning(CAT_EPROMTEST_API) << "Plugin not available";
		return;
	}
	m_plugin->switchDevice();
}

QString EpromTest_API::getBoard()
{
	if(!m_plugin) {
		qWarning(CAT_EPROMTEST_API) << "Plugin not available";
		return QString();
	}
	return m_plugin->board();
}

#include "moc_epromtest_api.cpp"

/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "saverestoretoolsettings.h"

#include <QSettings>

#include <pluginbase/apiobject.h>

using namespace scopy::m2k;

SaveRestoreToolSettings::SaveRestoreToolSettings(M2kTool *tool)
	: m_tool(tool)
{
	QSettings settings(m_temp.fileName(), QSettings::IniFormat);

	tool->getApi()->save(settings);

	settings.sync();
}

SaveRestoreToolSettings::~SaveRestoreToolSettings()
{
	QSettings settings(m_temp.fileName(), QSettings::IniFormat);

	m_tool->getApi()->load(settings);
}

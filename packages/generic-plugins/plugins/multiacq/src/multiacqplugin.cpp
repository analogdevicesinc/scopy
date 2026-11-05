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

#include "multiacqplugin.h"
#include "multiacqtool.h"
#include "scopy-multiacq_config.h"

#include <gui/style.h>
#include <gui/style_attributes.h>

using namespace scopy;
using namespace scopy::multiacq;

bool MultiAcqPlugin::compatible(QString param, QString category)
{
	Q_UNUSED(param)
	Q_UNUSED(category)
	return false;
}

QList<ToolMenuEntry *> MultiAcqPlugin::globalToolList()
{
	if(!m_toolList.isEmpty()) {
		return m_toolList;
	}
	ToolMenuEntry *tme = new ToolMenuEntry("multiacq", MULTIACQ_PLUGIN_DISPLAY_NAME,
					       ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
						       "/icons/tool_oscilloscope.svg",
					       m_name, QString(), this);
	tme->setEnabled(true);
	tme->setRunBtnVisible(true);
	m_tool = new MultiAcqTool(tme);
	tme->setTool(m_tool);
	m_toolList.append(tme);
	return m_toolList;
}

void MultiAcqPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}

QString MultiAcqPlugin::description() { return MULTIACQ_PLUGIN_DESCRIPTION; }

QString MultiAcqPlugin::pkgName() { return MULTIACQ_PKG_NAME; }

QString MultiAcqPlugin::about() { return MULTIACQ_PLUGIN_DESCRIPTION; }

QString MultiAcqPlugin::version() { return "0.1"; }

#include "moc_multiacqplugin.cpp"

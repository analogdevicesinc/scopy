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

#include "bareminimum.h"

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>

#include <pluginbase/messagebroker.h>
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN, "BareMinimum");
using namespace scopy;

bool BareMinimum::compatible(QString m_param, QString category)
{
	qDebug(CAT_TESTPLUGIN) << "compatible";
	return true;
}

void BareMinimum::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("bareminimum_tool", "MinimumTool",
						  ":/gui/icons/scopy-default/icons/tool_home.svg"));
}

bool BareMinimum::onConnect()
{
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setTool(new QLabel("Minimum"));
	return true;
}

bool BareMinimum::onDisconnect()
{
	m_toolList[0]->setEnabled(false);
	m_toolList[0]->setTool(nullptr);
	return true;
}

void BareMinimum::initMetadata() // not actually needed - putting it here to set priority
{
	loadMetadata(
		R"plugin(
	{
	   "priority":-255,
	   "category":[
	      "test"
	   ]
	}
)plugin");
}

#include "moc_bareminimum.cpp"

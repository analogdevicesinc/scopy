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

#include "guitestplugin.h"

#include <QBoxLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QSpacerItem>

Q_LOGGING_CATEGORY(CAT_GUITESTPLUGIN, "GUITestPlugin");
using namespace scopy;

bool TestPlugin::compatible(QString m_param, QString category) { return (m_param == "gui"); }

bool TestPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/guitestplugin/icon.png");
	return true;
}

bool TestPlugin::loadPage()
{
	m_page = new QWidget();
	return true;
}

void TestPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("Buttons", "Buttons", ":/gui/icons/home.svg"));
}

bool TestPlugin::onConnect()
{
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	tool = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(tool);
	QLabel *lbl = new QLabel("TestPlugin", tool);
	lay->addWidget(lbl);
	lay->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_toolList[0]->setTool(tool);

	return true;
}

bool TestPlugin::onDisconnect()
{
	qDebug(CAT_GUITESTPLUGIN) << "disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}
	return true;
}

void TestPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":2,
	   "category":[
	      "test"
	   ]
	}
)plugin");
}

#include "moc_guitestplugin.cpp"

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

#include "imuanalyzer.h"

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>

#include <pluginbase/messagebroker.h>
#include <pluginbase/preferences.h>
#include <qdebug.h>

#include <deviceiconbuilder.h>

Q_LOGGING_CATEGORY(CAT_IMUANALYZER, "IMUAnalyzer");
using namespace scopy;

bool IMUAnalyzer::compatible(QString m_param, QString category)
{
	qDebug(CAT_IMUANALYZER) << "compatible";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return ret;

	for(int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		std::string name = iio_device_get_name(dev);
		if(name.find("adis") != std::string::npos) {
			ret = true;
			goto finish;
		}
	}
finish:

	ConnectionProvider::GetInstance()->close(m_param);
	return ret;
}

void IMUAnalyzer::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("IMUAnalyzer_tool", "imuanalyzer",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_imuanalyzer.svg"));
}

bool IMUAnalyzer::onConnect()
{

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	if(conn == nullptr) {
		return false;
	}

	m_toolList[0]->setEnabled(true);

	m_imuInterface = new IMUAnalyzerInterface(m_param);

	QString tool_name = QString("IMUAnalyzer");

	m_toolList.last()->setTool(m_imuInterface);
	m_toolList.last()->setVisible(true);

	return true;
}

bool IMUAnalyzer::onDisconnect()
{
	m_toolList[0]->setEnabled(false);
	m_toolList[0]->setTool(nullptr);

	ConnectionProvider::GetInstance()->close(m_param);

	return true;
}

bool IMUAnalyzer::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
			  "/icons/tool_imuanalyzer.svg");
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("IMUAnalyzer");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
	return true;
}

void IMUAnalyzer::initMetadata() // not actually needed - putting it here to set priority
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}

#include "moc_imuanalyzer.cpp"

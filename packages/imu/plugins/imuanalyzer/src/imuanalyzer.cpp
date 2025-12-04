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

#include "imuanalyzer.h"

#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpacerItem>
#include <style.h>

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
		const char *rawName = iio_device_get_name(dev);
		std::string name = rawName ? rawName : "";
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("imuanalyzer", "IMUAnalyzer",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_imuanalyzer.svg"));
}

bool IMUAnalyzer::onConnect()
{

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	if(conn == nullptr) {
		return false;
	}
	ToolMenuEntry *imuTme = m_toolList[0];

	m_imuInterface = new IMUAnalyzerInterface(m_param);

	imuTme->setEnabled(true);
	imuTme->setTool(m_imuInterface);
	imuTme->setVisible(true);
	imuTme->setRunBtnVisible(true);

	connect(imuTme, &ToolMenuEntry::runToggled, m_imuInterface, &IMUAnalyzerInterface::runToggled);
	connect(m_imuInterface, &IMUAnalyzerInterface::runBtnPressed, imuTme, &ToolMenuEntry::setRunning);

	return true;
}

bool IMUAnalyzer::onDisconnect()
{
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
	m_imuInterface = nullptr;
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

QString IMUAnalyzer::description() { return "This plugin provides analysis and visualization for IMU sensor data"; }

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
